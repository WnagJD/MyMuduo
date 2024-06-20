#include "Channel.h"


#include "Logger.h"
#include "EventLoop.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kwriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop,  int fd):
    fd_(fd),loop_(loop),events_(0),revents_(0),index_(-1),tied_(false)
{

}

Channel::~Channel(){};


//注意tie调用时机
void Channel::tie(const std::shared_ptr<void>&obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::handleEvent(TimeStamp receiveTime)
{
    if(tied_)
    {
        //弱指针提升为强指针
        std::shared_ptr<void>guard = tie_.lock();
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(TimeStamp receiveTime)
{
    LOG_INFO("channel handleEvent receives:%d\n", revents_);
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(closecallback_)
        {
            closecallback_();
        }
    }

    if(revents_ & EPOLLERR)
    {
        if(errorcallback_)
        {
            errorcallback_();
        }
    }

    if(revents_ &(EPOLLIN | EPOLLPRI))
    {
        if(readcallback_)
        {
            readcallback_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT)
    {
        if(writecallback_)
        {
            writecallback_();
        }
    }
}
