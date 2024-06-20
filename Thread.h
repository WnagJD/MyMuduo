#pragma once

#include "nocopyable.h"


#include <functional>
#include <memory>
#include <thread>
#include <atomic>

//封装线程类
//将线程的启动封装成类的成员函数
class Thread : nocopyable
{
    public:
        //线程函数  虽然声明的线程函数只有一个参数,但是可以通过bind绑定参数来达到回调函数的传递
        using ThreadFunc = std::function<void()>;
        Thread(ThreadFunc, const std::string & name = std::string());
        ~Thread();

        void start();
        void join();

        bool started()const{return started_;}
        pid_t tid()const{return tid_;}
        const std::string& name()const{return name_;}

        static int numCreated(){return numCreated_;}


    private:

        void setDefaultName();
        //线程是否已经启动
        bool started_;
        //线程是否需要在调用线程中回收
        bool joined_; 

        //表示生成的线程 智能指针
        std::shared_ptr<std::thread> thread_;

        //获取线程的id
        pid_t tid_;

        //线程函数
        ThreadFunc func_;

        //给线程设置一个名字
        std::string name_;

        //记录全局生成的线程的数量
        static std::atomic_int numCreated_;
};