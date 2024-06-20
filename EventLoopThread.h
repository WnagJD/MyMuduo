#pragma once

#include "nocopyable.h"
#include "Thread.h"


#include <functional> 
#include <mutex>
#include <condition_variable>



class EventLoop;

//one loop per thread
class EventLoopThread : nocopyable
{
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;
        EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
            const std::string& name = std::string());
        
        ~EventLoopThread();

        //开启loop循环
        EventLoop* startLoop();

    private:

        void threadFunc();
        //保存在线程中声明的EventLoop
        EventLoop* loop_;
        bool exiting_;
        //线程对象
        Thread thread_;
        std::mutex mutex_;
        std::condition_variable cond_;
        //线程初始化的执行的回调函数
        ThreadInitCallback callback_;




};