#include "Poller.h"
#include "Channel.h"

Poller::Poller(EventLoop * loop):ownerLoop_(loop)
{

}


//判断参数channel是否在poller中
bool Poller::hasChannel(Channel* channel) const
{
    int fd  = channel->fd();
    auto it = channels_.find(fd);
    // if(it!= channels_.end() && it->second == channel)
    // {
    //     return true;
    // }

    // return false;

    return it!=channels_.end() && it->second == channel;


}
