#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"


#include <sys/eventfd.h>
#include <functional>
#include <unistd.h>
//防止一个线程中创建多个EventLoop ===> (one loop per thread)
__thread EventLoop* t_loopInThisThread = nullptr;

//定义默认的poller的I/O复用的超时时间
const int kPollTimeMs = 10000;

//创建wakeupFd,用来notify唤醒subReactor处理新的channel
//eventfd ==> 事件文件描述符
int createEventFd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd <0)
    {
        LOG_FATAL("eventfd error:%d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    :looping_(false)
    ,quit_(false)
    ,callPendingFunctors_(false)
    ,threadId_(CurrentThread::tid())
    ,poller_(Poller::newDefaultPoller(this))
    ,wakeupFd_(createEventFd())
    ,wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Anthor EventLoop %p exists in this thread %d \n", this, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    //设置wakeupfd的事件类型以及发生事件后的回调
    wakeupChannel_->enableReading();
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));

}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

//开启事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    
    LOG_INFO("EventLoop %p statr looping \n", this);
    while(!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for(Channel* channel : activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }

        doPendingFunctors();
    }
}



//退出事件循环
void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeup();
    }
    
}

//在当前loop中执行
void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
    
}
//把cb放入队列中,唤醒loop所在的线程,执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    //唤醒loop所在的线程
    if(!isInLoopThread() || callPendingFunctors_)
    {
        wakeup();
    }

}
    
//用来唤醒loop所在的线程
void EventLoop::wakeup()
{
    uint64_t one =1;
    ssize_t n = write(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes insead of 8", n);
    }
    
}
//EventLoop => 使用Pollerz中的方法
void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    poller_->hasChannel(channel);
}



void EventLoop::handleRead()
{
    uint64_t one =1;
    ssize_t n = read(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one))
    {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead od 8", n);
    }

}
void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const Functor &cb : functors)
    {
        cb();//执行当前loop需要执行的回调操作
    }
    callPendingFunctors_ = false;

}
