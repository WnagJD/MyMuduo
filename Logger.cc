#include "Logger.h"
#include <iostream>
#include "Timestamp.h"

//设置日志级别
void Logger::SetLogLevel(int level)
{
    logLevel_ = level;
}

//写日志
void Logger::Log(std::string message)
{
    switch(logLevel_)
    {
        case INFO: std::cout<<"[INFO]";break;
        case ERROR: std::cout<<"[ERROR]";break;
        case FATAL: std::cout<<"[FATAL]";break;
        case DEBUG: std::cout<<"[DEBUG]";break;
        default: break;
    }
    //打印时间和msg
    std::cout<<TimeStamp::now().ToString()<<message<<std::endl;
}

//获取Logger的单例实体
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;

}