#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    //thread_local
    //__thread 代表线程本地:每个线程都会维护一个自己的t_cachedTid变量
    extern __thread int t_cachedTid;

    void cacheTid();
    
    inline int tid()
    {
        if(__builtin_expect(t_cachedTid ==0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}