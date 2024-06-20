#pragma once



//继承nocopyable的派生类,可以正常的构造和析构
//但是不能进行拷贝构造和赋值操作,因为基类的这两种操作已经被禁止
class nocopyable
{
    public:
        nocopyable(const nocopyable&) = delete;
        nocopyable& operator=(const nocopyable&) = delete;
    protected:
        nocopyable()=default;
        ~nocopyable()=default;


};