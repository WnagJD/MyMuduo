#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
        :loop_(nullptr)
        ,thread_(std::bind(&EventLoopThread::threadFunc, this), name)
        ,exiting_(false)
        ,callback_(cb)
        ,mutex_()
        ,cond_()
{

}
        
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_!=nullptr)
    {
        loop_->quit();
        thread_.join();
    }

}

//开启loop循环  
//将
EventLoop* EventLoopThread::startLoop()
{
    thread_.start();
    EventLoop* loop =nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;

}
void EventLoopThread::threadFunc()
{

    //定义EventLoop
    EventLoop loop;
    //线程初始化执行的回调函数
    if(callback_)
    {
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex>lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();

    }
    loop.loop(); //EventLoop::loop ==> Poller::poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}