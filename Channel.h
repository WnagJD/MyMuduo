#pragma once
#include <functional>
#include <memory>
#include <sys/epoll.h>

#include "nocopyable.h"
#include "Timestamp.h"

//类型的前置声明
class EventLoop;

class Channel : nocopyable
{
    public:
        using ReadEventCallback = std::function<void(TimeStamp)>;
        using EventCallback = std::function<void(void)>;
    
    Channel(EventLoop* loop, int fd);
    ~Channel();

    //fd得到poller通知后,处理事件的函数
    void handleEvent(TimeStamp receiveTmie);

    //设置回调函数对象 std::move 将左值转化为右值
    void setReadCallback(ReadEventCallback cb){readcallback_ = std::move(cb);}
    void setWriteCallback(EventCallback cb){writecallback_ = std::move(cb);}
    void setCloseCallback(EventCallback cb){closecallback_ = std::move(cb);}
    void setErrorCallback(EventCallback cb){errorcallback_ = std::move(cb);}

    //防止channel被手动remove掉时,channel还在执行回调函数
    void tie(const std::shared_ptr<void>&);

    int fd() const {return fd_;}
    int events()const {return events_;}
    int set_revents(int revt){revents_ = revt;}

    //设置fd感兴趣的事件状态
    void enableReading(){events_ |= EPOLLIN; update();}
    void disableReading(){events_ &= ~EPOLLIN; update();}
    void enableWriting(){events_ |= EPOLLOUT; update();}
    void disableWriting(){events_ &= ~EPOLLOUT; update();}
    void disableAll(){events_ =kNoneEvent; update();}

    //返回fd当前的事件状态
    bool isNoneEvent(){return events_ == kNoneEvent;}
    bool isReadEvent(){return events_ == kReadEvent;}
    bool isWriteEvent(){return events_ == kwriteEvent;}

    int index(){return index_;}
    void set_index(int idx){index_ = idx;}


    //返回当前的channel属于哪一个EventLoop
    EventLoop* ownerloop(){return loop_;}
    void remove();

    private:

        void update();
        void handleEventWithGuard(TimeStamp receiveTime);
        static const int kNoneEvent;//没有感兴趣的事件
        static const int kReadEvent;
        static const int kwriteEvent;

        EventLoop* loop_; //事件循环
        const int fd_; //fd,poller监听的对象
        int events_; //fd感兴趣的事件
        int revents_; // poller 返回具体的发生事件
        int index_;

        //弱智能指针
        std::weak_ptr<void> tie_;
        bool tied_;


        //根据poller的通知,Channel执行回调函数
        ReadEventCallback readcallback_;
        EventCallback writecallback_;
        EventCallback closecallback_;
        EventCallback errorcallback_;

};