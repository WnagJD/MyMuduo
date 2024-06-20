#include "TcpServer.h"
#include "Logger.h"


#include <strings.h>

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d ==> mainloop is null \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}


TcpServer::TcpServer(EventLoop* loop,const InetAddress&listenaddr,
                    const std::string & nameArg, Option option)
                    :loop_(CheckLoopNotNull(loop))
                    ,ipPort_(listenaddr.toIpPort())
                    ,name_(nameArg)
                    ,acceptor_(new Acceptor(loop,listenaddr, option))
                    ,threadpool_(new EventLoopThreadPool(loop,nameArg))
                    ,connectionCallback_()
                    ,messageCallback_()
                    ,nextConnId_(1)
                    ,started_(0)

{
    //当有新用户连接时,TcpServer会首先调用newconnection
   acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, 
                            this,std::placeholders::_1, std::placeholders::_2));


}

TcpServer::~TcpServer()
{
    for(auto& item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();

        //调用的是TcpConnection::connectionDestoryed
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
    }

}

//设置底层subloop线程数
void TcpServer::setThreadNums(int numThreads)
{
    threadpool_->setThreadNum(numThreads);

}

//开启服务器监听
void TcpServer::start()
{
    if(started_++ ==0) //防止一个TcpServer被多次调用
    {
        threadpool_->start(threadinitCallback_); //启动底层线程池
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
        
    }
}


//当有新的客户端连接的时候,acceptor会执行这个回调
void TcpServer::newConnection(int sockfd, const InetAddress& peeraddr)
{
    //通过轮询算法,选择一个subloop
    EventLoop * loop = threadpool_->getnextLoop();
    char buf[64]={0};
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(),nextConnId_);
    ++nextConnId_ ;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection[%s] - new connection [%s] from %s \n",
                name_.c_str(), connName.c_str(), peeraddr.toIpPort().c_str());

    //通过sockfd获取绑定本机ip和端口
    struct sockaddr_in localaddr;
    socklen_t len = sizeof(localaddr);
    bzero(&localaddr, len);
    if(::getsockname(sockfd,(sockaddr*)&localaddr,&len)<0)
    {
        LOG_ERROR("sockets:getLocalAddr \n");

    }
    InetAddress localAddr(localaddr);

    //建立Tcpconnection
    TcpConnectionPtr conn(new TcpConnection(loop,connName,sockfd,localAddr,peeraddr));

    connections_[connName]=conn;

    //给TcpConnection设置回调函数
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writecompleteCallback_);


    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, conn));

    loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));





}
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{

    loop_->runInLoop(std::bind(&TcpServer::removeConnectionLoop, this, conn));
}

//底层调用的是TcpConnection::connectionDestoryed
void TcpServer::removeConnectionLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO("TcpServer::removeConnection [%s] - connection [%s] \n", name_.c_str(), conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop * ioloop = conn->getLoop();

    ioloop->queueInLoop(std::bind(&TcpConnection::connectDestoryed,conn)); 
}