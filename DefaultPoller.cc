#include "Poller.h"
#include "EpollPoller.h"

#include <stdlib.h>


//EventLoop 可以通过该接口获取默认的IO复用的具体实现
Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;
    }
    else
    {
        //生成epoll的实例
        return new EpollPoller(loop);
    }

}