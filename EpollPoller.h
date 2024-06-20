#pragma once
#include "Poller.h"


#include <sys/epoll.h>

class Channel;

class EpollPoller: public Poller
{
    public:
        EpollPoller(EventLoop* loop);
        ~EpollPoller() override;  //override 检查此方法为抽象方法的重写

        //重写基类Poller的抽象方法
        TimeStamp poll(int timeoutMs, ChannelList* activeChannels) override;
        void updateChannel(Channel* channel) override;
        void removeChannel(Channel* channel) override;

    private:
        void update(int operation, Channel* channel);
        void fillActiveChannels(int num, ChannelList* activeChannels);

        static const int kInitEventListSize =16;
        int epollfd_;
        
        using EventList = std::vector<epoll_event>;
        EventList events_;

};
