#include "TcpConnection.h"
#include "Timestamp.h"
#include "InetAddress.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"


#include <functional>

//检查传入的EventLoop指针不为空
static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
         LOG_FATAL("%s:%s:%d ==> mainloop is null \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}


TcpConnection::TcpConnection(EventLoop* loop, const std::string& nameArgs
                                , int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
                                :loop_(CheckLoopNotNull(loop))
                                ,name_(nameArgs)
                                ,state_(kConnecting)
                                ,reading_(true)
                                ,localAddr_(localAddr)
                                ,peerAddr_(peerAddr)
                                ,socket_(new Socket(sockfd))
                                ,channel_(new Channel(loop,sockfd))
                                ,highwaterMark_(64*1024*1024) //64MB
    {
        //给生成的channel设置回调函数
        channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
        channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
        channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
        channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

        LOG_INFO("TcpConnection::ctor[%s] at sockfd =%d \n", name_.c_str(), sockfd);
        socket_->setKeepActive(true);

    }
TcpConnection::~TcpConnection()
{
    LOG_INFO("Tcoconection::dtor[%s] at fd=%d state =%d \n", name_.c_str(), channel_->fd(), (int)state_);
}

 //发送数据
void TcpConnection::send(const std::string& buf)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            loop_->queueInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote =0;
    size_t remaining =len;
    bool faultError = false;

    
    if(state_ == kDisconnected)
    {
        LOG_ERROR("TcpConnection::disconnected, give up writing \n");
        return ;
    }

    //当第一次发送数据
    if(!channel_->isWriteEvent() && outputBuffer_.readableBytes()==0)
    {
        int nwrote = ::write(channel_->fd(),data, len);
        if(nwrote >=0)
        {
            remaining = len - nwrote;
            if(remaining ==0 && writecompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writecompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrote =0;
            if(errno!= EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendLoop");
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    

    //当send发送数据没有完全发送的时候,将数据保存在outputBuffer_中
    //给poller注册EPOLLOUT事件,使poller监测fd上的可写事件,并且将数据不断的从outputBuffer_中写入fd对应的Tcp连接中
    //当数据完全写完的时候,架构EPOLLOUT事件从poller中删除
    //应用写数据很快并且数据量很大    内核写数据慢   保持应用和内核的速率,使用Buffer缓冲区
    if(!faultError && remaining >0)
    {
        size_t oldlen = outputBuffer_.readableBytes();
        if(oldlen + remaining >= highwaterMark_
            && oldlen < highwaterMark_ 
            && highwaterCallback_)
            {
                loop_->queueInLoop(std::bind(writecompleteCallback_, shared_from_this()));
            }
        
        outputBuffer_.append((char*)data + nwrote, remaining);
        if(!channel_->isWriteEvent())
        {
            //向poller中注册EPOLLOUT 事件
            channel_->enableWriting();
        }
    }

}


//关闭连接
void TcpConnection::shutdwon()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);//设置状态为将要关闭
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriteEvent())//表示outputBuffer_中的数据已经全部被发送出去,调用了channel_->diswriting()
    {
        socket_->shutdownWrite();
    }

}


//建立连接
void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();//添加写事件


    //执行建立连接的回调函数
    connectionCallback_(shared_from_this());

}

//连接销毁
void TcpConnection::connectDestoryed()
{
    if(state_ ==kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        //调用回调函数
        connectionCallback_(shared_from_this());
    }

    channel_->remove();
}


void TcpConnection::handleRead(TimeStamp receiveTime)
{
    int saverrno =0;
    //从sockfd读取数据到Buffer里
    ssize_t num = inputBuffer_.readFd(channel_->fd(),&saverrno);
    if(num >0)
    {
        messageCallback_(shared_from_this(),&inputBuffer_, receiveTime);
    }
    else if(num == 0)
    {
        handleClose();  //细节
    }
    else{
        errno = saverrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
     

}
void TcpConnection::handleWrite()
{
    if(channel_->isWriteEvent())
    {
        int savedErrno =0;
        int num = outputBuffer_.writeFd(channel_->fd(),&savedErrno);
        if(num >0) //成功写入
        {
            outputBuffer_.retrieve(num);
            if(outputBuffer_.readableBytes() == 0) //缓冲区中的数据被写完了
            {
                channel_->disableWriting(); //删除poller上的写事件监听
                if(writecompleteCallback_)
                {
                    //做函数的绑定
                    loop_->queueInLoop(std::bind(writecompleteCallback_, shared_from_this()));
                }

                if(state_ == kDisconnecting)
                {
                   shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection:handlewrite");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down, no moew wwriting \n", channel_->fd());
    }
}
void TcpConnection::handleClose()
{
    LOG_INFO("TcpConnection::handleclose fd=%d state=%d \n", channel_->fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();//将poller上注册的事件全部取消

    TcpConnectionPtr conn(shared_from_this());
    connectionCallback_(conn); //执行连接回调函数
    closeCallback_(conn); //执行关闭回调函数
}
void TcpConnection::handleError()
{
    int optval;
    socklen_t len = sizeof(optval);
    int err = 0;
    if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR, &optval, &len)<0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcoConnection::handleError name:%s -SO_ERROR:%d \n", name_.c_str(), err);
}


