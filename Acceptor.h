#pragma once

#include "nocopyable.h"
#include "Socket.h"
#include "Channel.h"


#include <functional>


class InetAddress;
class Eventloop;



class Acceptor : nocopyable
{
    public:
        using NewConnectionCallback = std::function<void(int sockfd, const InetAddress& addr)>;
        Acceptor(EventLoop *loop,const InetAddress& addr, bool reuseport);

        ~Acceptor();


        void setNewConnectionCallback(const NewConnectionCallback& cb)
        {
            newConnectionCallback_ = cb;
        }

        bool listening()const{return listening_;}
        void listen();

    private:
        void handleRead();

        EventLoop* loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback newConnectionCallback_;
        bool listening_;


};