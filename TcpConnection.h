#pragma once

#include "nocopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"


#include <string>
#include <atomic>
#include <memory>

class TimeStamp;
class EventLoop;
class Channel;
class Socket;


//std::enable_shared_from_this<xxx> 模板类, 自定义的类通过公有继承此模板类
//可以在类中使用shared_from_this()函数来安全的生成指向当前对象的智能指针
class TcpConnection : nocopyable, public std::enable_shared_from_this<TcpConnection>
{

public:
    TcpConnection(EventLoop* loop, const std::string& nameArgs
                                , int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    //获取当前的TcpConnection属于的subloop
    EventLoop* getLoop() const{return loop_;}
    const std::string& name()const{return name_;}
    const InetAddress& localAddr()const{return localAddr_;}
    const InetAddress& peerAddr()const{return peerAddr_;}

    bool connected(){return state_ == kConnected;}

    void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback& cb){messageCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){writecompleteCallback_ = cb;}
    void setCloseCallback(const CloseCallback& cb){closeCallback_ = cb;}

    //发送数据
    void send(const std::string& buf);

    //关闭连接
    void shutdwon();

    //建立连接
    void connectEstablished();

    //连接销毁
    void connectDestoryed();

   
private:
    
    enum STATE{kDisconnected, kConnecting, kConnected, kDisconnecting};
    void setState(STATE state){state_ = state;}

    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();

    //绝对不是mainloop,而是通过轮询算法选出的subloop
    EventLoop* loop_;
    std::string name_;
    std::atomic_int state_; //TcpConnection的状态
    bool reading_;

    //通过TcpConnection 封装出对应的Channel
    std::shared_ptr<Socket> socket_;
    std::shared_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    //回调函数
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writecompleteCallback_;
    HighWaterCallback highwaterCallback_;
    CloseCallback closeCallback_;
    size_t highwaterMark_;//高水位的界限


    Buffer inputBuffer_;//读缓冲
    Buffer outputBuffer_;//写缓冲
};