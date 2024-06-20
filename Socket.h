#pragma once

#include "nocopyable.h"

class InetAddress;

class Socket : nocopyable
{
public:
    explicit Socket(int sockfd):
        sockfd_(sockfd)
        {

        }
    ~Socket();

    int fd()const{return sockfd_;}
    void bind(const InetAddress& addr);
    void listen();
    int accept(InetAddress* peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReusePort(bool on);
    void setReuseAddr(bool on);
    void setKeepActive(bool on);

private:
    const int sockfd_;

};