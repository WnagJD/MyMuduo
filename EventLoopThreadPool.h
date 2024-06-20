#pragma once

#include "nocopyable.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : nocopyable
{
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;

        EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArgs);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads){numThreads_ = numThreads;}

        void start(const ThreadInitCallback& cb = ThreadInitCallback());

        //如果工作在多线程,base_loop_默认以轮询的方式分配channel
        EventLoop* getnextLoop();

        std::vector<EventLoop*> getAllLoops();

        bool started()const{return started_;}
        const std::string name()const{return name_;}



    private:

        //保存线程产生的EventLoop
        std::vector<EventLoop*> loops_;
        //保存产生的EventLoopThread
        std::vector<std::shared_ptr<EventLoopThread>> threads_;

        int next_;

        EventLoop* baseloop_;
        std::string name_;

        bool started_;
        //设置的线程数
        int numThreads_;

        
};