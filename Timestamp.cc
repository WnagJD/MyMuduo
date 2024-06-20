#include "Timestamp.h"
#include <time.h>

//构造函数
TimeStamp::TimeStamp():microSecondSinceEpoch_(0)
{
}

//必须显式的调用此构造函数(避免隐式构造函数的默认转换,导致未知的错误)
TimeStamp::TimeStamp(int64_t microSecondSinceEpoch):microSecondSinceEpoch_(microSecondSinceEpoch)
{

}

TimeStamp TimeStamp::now()
{
    return TimeStamp(time(nullptr));
}

std::string TimeStamp::ToString() const
{
    char buf[128]={0};
    struct tm* tim = localtime(&microSecondSinceEpoch_);
    snprintf(buf,128,"%4d/%02d/%02d %02d:%02d:%02d",
                tim->tm_year+1900,
                tim->tm_mon+1,
                tim->tm_mday,
                tim->tm_hour,
                tim->tm_min,
                tim->tm_sec);

    return buf;
}


// int main()
// {
//     std::cout<<TimeStamp::now().ToString()<<std::endl;
//     return 0;
// }