#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"


#include <errno.h>
#include <unistd.h>
#include <strings.h>

//表示channel还没添加的epoll中 
const int kNew =-1;
//表示channel已经添加到epoll中,并且存在感兴趣的事件
const int KAdded =1;
//表示channel从epoll中删除(相当于没有监听),但是依旧存在于channels_(channelMap类型)
const int KDeleted =2;


EpollPoller::EpollPoller(EventLoop* loop)
    :Poller(loop)
    ,epollfd_(epoll_create1(EPOLL_CLOEXEC))
    ,events_(kInitEventListSize)
{
    if(epollfd_<0)
    {
        LOG_FATAL("epoll_create error: %d \n", errno);
    }

}


EpollPoller::~EpollPoller()//override 检查此方法为抽象方法的重写
{
    ::close(epollfd_);

}

//重写基类Poller的抽象方法
TimeStamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG_INFO("fun%s ==> fd toatl count:%lu \n", __FUNCTION__, events_.size());
    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    TimeStamp now(TimeStamp::now());
    if(numEvents>0)
    {
        LOG_INFO("%d events happend \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);

        if(numEvents == events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvents ==0)
        {
            LOG_DEBUG("%s timeout! \n", __FUNCTION__);
        }
    else
        {
            if(saveErrno != EINTR)
            {
                errno = saveErrno;
                LOG_ERROR("EpollPoller::poll error \n");
            }

        }
    return now;

}

//updateChannel 和 removeChannel 都是判断channel中的内容进行执行的(符合框架类型的设计,
//尽可能在暴露在外部的接口的使用,不会触及底层的知识,方便编程人员的使用)

//可能得情况
/*
channels_ 和 poller上添加的fd不完全相同,channels_ >= poller
当某个channel中的events的没有感兴趣的事件的时候，从poller中删除，但是还没从channels_中删除


*/ 
void EpollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__, channel->fd(), channel->events(), channel->index());

    if(index == kNew || index == KDeleted) //添加事件
    {
        if(index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        
        channel->set_index(KAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(KDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }


}
void EpollPoller::removeChannel(Channel* channel)
{

    //分两种情况
    //一种是处于KDeleted
    //一种是处于KAdded
    int fd = channel->fd();
    auto it = channels_.find(fd);
    if(it!=channels_.end())
    {
        channels_.erase(fd);
    }

    LOG_INFO("func=%s => fd=%d \n", __FUNCTION__, fd);

    int index = channel->index();
    if(index == KAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }

    channel->set_index(kNew);

}


void EpollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    if(::epoll_ctl(epollfd_,operation, channel->fd(),&event)<0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl_del error:%d \n", errno);
        }
        else
        {
            LOG_FATAL("epocc_ctl_add/mod error:%d \n", errno);
        }
    }
   

}
void EpollPoller::fillActiveChannels(int num, ChannelList* activeChannels)
{
    for(int i=0;i<num;i++)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        //EventLoop获取poller返回的触发事件
        activeChannels->push_back(channel);
    }

}