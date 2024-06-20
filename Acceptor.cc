#include "Acceptor.h"
#include "EventLoop.h"
#include "Logger.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Logger.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

static int createNonblocking()
{
        //创建用于监听的套接字，并设置套接字为非阻塞
        int sockfd = ::socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,0);
        if(sockfd <0)
        {
                LOG_FATAL("%s:%s:%d => crerate sockfd error! \n", __FILE__, __FUNCTION__, __LINE__);
        }

        return sockfd;
}


 Acceptor::Acceptor(EventLoop *loop,const InetAddress& addr, bool reuseport)
        :loop_(loop)
        ,acceptSocket_(createNonblocking())
        ,acceptChannel_(loop_, acceptSocket_.fd())
        ,listening_(false)
 {
        acceptSocket_.setReusePort(true);
        acceptSocket_.setReuseAddr(true);
        acceptSocket_.bind(addr);

        acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
 }

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();  //listen
    acceptChannel_.enableReading();  //acceptChannel ==> poller
}


void Acceptor::handleRead()
{
    InetAddress peer;
    int connfd = acceptSocket_.accept(&peer);
    if(connfd >=0)
    {
        if(newConnectionCallback_)
        {
            newConnectionCallback_(connfd,peer);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d ==> accept error \n",__FILE__,__FUNCTION__,__LINE__);
        if(errno == EMFILE)
        {
            //fd的分配达到进程所具有资源的最大
            LOG_ERROR("%s:%s:%d ==> sockfd reached limit! \n",__FILE__,__FUNCTION__,__LINE__);
        }
    }

}