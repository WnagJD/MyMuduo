#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

//向fd读取数据

//无法确定fd中具有多少的数据
//使用额外的空间保存
//readv 可以将从fd 中读取到的数据放到不连续的空间中
//  struct iovec 
//  {
//     void* iov_base;
//     size_t len;
//  }
ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf[65536] ={0};
    struct iovec vec[2];

    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = writable < sizeof(extrabuf) ? 2:1;

    ssize_t num = ::readv(fd, vec, iovcnt);

    if(num < 0)
    {
        *saveErrno = errno;
    }
    else if(num <=writable) //读取的数据writable大小的空间已经足够
    {
        writerIndex_ +=num;
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, num - writable);
    }
    return num;
     
}




//向fd中写数据
ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t num = ::write(fd,peek(),readableBytes());
    if(num <0)
    {
        *saveErrno = errno;
    }

    return num;
}