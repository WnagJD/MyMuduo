#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>


Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bind(const InetAddress& addr)
{
    if(0 != ::bind(sockfd_,(struct sockaddr*)addr.getSockAddr(),(socklen_t)sizeof(*addr.getSockAddr())))
    {
        LOG_FATAL("%s:%s:%d=> sockfd bind error \n", __FILE__, __FUNCTION__, __LINE__);
    }
    
}
void Socket::listen()
{
    if(0!= ::listen(sockfd_,1024))
    {
        LOG_FATAL("%s:%s:%d=> sockfd listen error \n", __FILE__, __FUNCTION__, __LINE__);
    }
}
int Socket::accept(InetAddress* peeraddr)
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);//传入传出参数
    bzero(&addr,sizeof(addr));
    int confd = ::accept4(sockfd_,(sockaddr*)&addr,&len,SOCK_NONBLOCK | SOCK_CLOEXEC); //新生成的fd是非阻塞的

    if(confd >=0)
    {
        peeraddr->SetScokaddr(addr);
    }

    return confd;

}

void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_,SHUT_WR)<0)
    {
        LOG_FATAL("%s:%s:%d=> shutdownWriter error \n", __FILE__, __FUNCTION__, __LINE__);
    }
}

//设置数据立即发送
void Socket::setTcpNoDelay(bool on)
{
    int optval = on? 1:0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval, sizeof optval);
}

//设置端口复用
void Socket::setReusePort(bool on)
{
    int optval = on? 1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&optval, sizeof optval);
}
// 设置IP复用
void Socket::setReuseAddr(bool on)
{
    int optval = on? 1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval, sizeof optval);
}

// 设置保持连接/活跃
void Socket::setKeepActive(bool on)
{
    int optval = on? 1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval, sizeof optval);
}