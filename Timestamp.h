#pragma once

#include <iostream>


//从逻辑上来分析,封装的TimeStamp类是用于工具方法(起到类似于函数的功能),
//因此封装此类的时候,尽可能的不在每次使用的时候,都去声明一个实体对象,再来调用对应的方法
//或则是将对象的定义写在成员函数中
class TimeStamp
{
    public:
        //构造函数
        TimeStamp();

        //必须显式的调用此构造函数(避免隐式构造函数的默认转换,导致未知的错误)
        explicit TimeStamp(int64_t microSecondSinceEpoch);

        static TimeStamp now();

        std::string ToString() const;

    private:
        int64_t microSecondSinceEpoch_;


};