#pragma once

#include <string>
#include <vector>
#include <algorithm> //泛型函数 --- 模版函数

//网络库底层的缓冲区类型定义


//Buffer 是面向应用层的, 作为一个中间层, 可保存从fd读取到的数据，也可将要写入的数据放入buffer中
//Buffer 是缓冲区,在具体的场景中,可以分为是读缓冲或者是写缓冲
//Buffer中的数据,是通过readerIndex 和 writerIndex 来区分的
//Index 来区分有数据还是没有数据


//可读和可写是针对Buffer的底层空间所说的  体现了在一定的空间中有数据的部分和没有数据的部分


class Buffer
{

public:
    static const size_t kCheapPrepend =8;
    static const size_t kInitialSize =1024;

    explicit Buffer(size_t initialSize = kInitialSize)
                    :buffer_(kCheapPrepend + initialSize)
                    ,readerIndex_(kCheapPrepend)
                    ,writerIndex_(kCheapPrepend)
                {

                }

    //可读的字节数            
    size_t readableBytes()const
    {
        return writerIndex_ - readerIndex_;

    }

    //可写的字节数
    size_t writableBytes()const
    {
        return buffer_.size() - writerIndex_;
    }
    
    //可读索引之前的字节数
    size_t prependableBytes()const
    {
        return readerIndex_;
    }

    //返回缓冲区可读数据的地址
    const char* peek()const
    {
        return begin() + readerIndex_;
    }


    //读取buffer_中可读区间的数据
    void retrieve(size_t len)
    {
        if(len < readableBytes())
        {
            readerIndex_ += len;
        }
        else //包括len == readableBytes()
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    //把onmessage回调函数上报Buffer的数据转换成string类型的
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);//进行复位操作
        return result;
    }

    //确保有可写的空间
    void ensureWritableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            //buffer_扩容操作
            makeSpace(len);
        }
    }

    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const 
    {
        return begin() + writerIndex_;
    }

    //从fd中读取数据
    ssize_t readFd(int fd, int* saveErrno);
    //向fd中写数据
    ssize_t writeFd(int fd, int* saveErrno);


private:

    char* begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const 
    {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len)
    {
        //判断当前的buffer_空闲的空间和writableBytes()的空间的和是否能够满足len大小
        if(prependableBytes() + writableBytes() < len + kCheapPrepend)
        {
            //buffer_中空闲的空间不能满足len
                buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_
                    ,begin() + writerIndex_
                    ,begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }


    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;



};