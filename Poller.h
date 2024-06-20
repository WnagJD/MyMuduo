#pragma once
#include "nocopyable.h"
#include "Timestamp.h"


#include <vector>
#include <unordered_map>

//类型的前置声明
class Channel;
class EventLoop;

//poller是抽象基类
/*
                   poller
        pollpoller        epollpoller

*/


class Poller : nocopyable
{
    public:
        using ChannelList = std::vector<Channel*>;

        Poller(EventLoop * loop);
        virtual ~Poller() = default;

        //给所有的I/O复用保留统一的接口
        //epoll_wait  纯虚函数
        virtual TimeStamp poll(int timeoutMs, ChannelList* activeChannels) =0;
        
        //epoll_ctl
        virtual void updateChannel(Channel* channel) =0;
        virtual void removeChannel(Channel* channel) =0;


        //判断参数channel是否在poller中
        bool hasChannel(Channel* channel) const;

        //EventLoop 可以通过该接口获取默认的IO复用的具体实现
        static Poller* newDefaultPoller(EventLoop* loop);

    protected:    
        using ChannelMap = std::unordered_map<int,Channel*>;
        //保存poller上监听的fd和事件
        ChannelMap channels_;
    private:
        EventLoop* ownerLoop_;
        

};