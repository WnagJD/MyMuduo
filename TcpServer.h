#pragma once

#include "nocopyable.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "Buffer.h"


#include <functional>
#include <string>
#include <unordered_map>
#include <atomic>



class TcpServer : nocopyable
{

public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;

        enum Option{
            kNOReusePort,
            kReusePort
        };

        TcpServer(EventLoop* loop,const InetAddress&listenaddr,const std::string & nameArg, Option option = kNOReusePort);
        ~TcpServer();

        void setThreadInitCallback(const ThreadInitCallback& cb){ threadinitCallback_ = cb;}
        void setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = cb;}
        void setMessageCallback(const MessageCallback& cb){messageCallback_ = cb;}
        void setWriteCompletecallback(const WriteCompleteCallback& cb){ writecompleteCallback_ = cb;}

        //设置底层subloop线程数
        void setThreadNums(int numThreads);

        //开启服务器监听
        void start();

private:

        void newConnection(int sockfd, const InetAddress& peeraddr);
        void removeConnection(const TcpConnectionPtr& conn);
        void removeConnectionLoop(const TcpConnectionPtr& conn);

        using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

        EventLoop* loop_;//mainloop

        const std::string ipPort_;
        const std::string name_;

        std::unique_ptr<Acceptor> acceptor_;
        std::shared_ptr<EventLoopThreadPool> threadpool_;


        //新连接的回调
        ConnectionCallback connectionCallback_;

        //有读写消息的回调
        MessageCallback messageCallback_;

        //消息发送完之后的回调
        WriteCompleteCallback writecompleteCallback_;

        //线程初始化的回调
        ThreadInitCallback threadinitCallback_;

        std::atomic_int started_;

        int nextConnId_;

        //保存所有的连接
        ConnectionMap connections_;


};