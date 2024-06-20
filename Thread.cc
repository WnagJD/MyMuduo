#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

//std::atomic_int 类类型
std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string & name)
        :started_(false)
        ,joined_(false)
        ,tid_(0)
        ,name_(name)
        ,func_(std::move(func))
{    
    setDefaultName();
}

Thread::~Thread()
{
    if( started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem,0,0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        
        //获取线程id
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    }));

    //保证生成的线程获取到tid_,接下来可以进行判断tid_的值
    sem_wait(&sem);

}
void Thread::join()
{
    joined_ = true;
    thread_->join();
    
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32]={0};
        snprintf(buf, sizeof(buf),"Thread%d", num);
        name_ = buf;

    }
}