#pragma once

#include "nocopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>


class Channel;
class Poller;


//事件循环类 包括两大模块 Poller  Channel
class EventLoop : nocopyable
{
    public:
        using Functor = std::function<void()>;
        EventLoop();
        ~EventLoop();

        //开启事件循环
        void loop();

        //退出事件循环
        void quit();

        //poller返回事件的时间
        TimeStamp pollReturnTime()const{return pollReturnTime_;}

        //在当前loop中执行
        void runInLoop(Functor cb);

        //把cb放入队列中,唤醒loop所在的线程,执行cb
        void queueInLoop(Functor cb);

        //用来唤醒loop所在的线程
        void wakeup();

        //EventLoop => 使用Pollerz中的方法
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);

        //判断EventLoop 是否在自己的线程里面
        bool isInLoopThread()const{return threadId_ == CurrentThread::tid();}











    private:
        void handleRead();//wakeupfd 事件的回调函数
        void doPendingFunctors(); //执行回调

        std::atomic_bool looping_; //原子操作:对这个变量的操作是原子类型的  表示loop开启的标致
        std::atomic_bool quit_;//标识退出loop循环

        TimeStamp pollReturnTime_;
        std::unique_ptr<Poller> poller_;
        const pid_t threadId_;//记录当前loop所在的线程id

        using ChannelList = std::vector<Channel*>;
        ChannelList activeChannels_;

        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;

        std::atomic_bool callPendingFunctors_;//标识当前loop是否有需要执行的回调操作,是否正在执行PendingFunctors
        std::vector<Functor> pendingFunctors_;//存储loop需要执行的回调操作
        std::mutex mutex_; //互斥锁  在多线程中用来保护pendingFunctors_的线程安全操作

};