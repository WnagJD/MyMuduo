#pragma once //编译器级别
#include "nocopyable.h"
#include <string>

#define LOG_INFO(logFormatMsg,...)\
    do\
    {\
        Logger& logger = Logger::GetInstance();\
        logger.SetLogLevel(INFO);\
        char buf[1024]={0};\
        snprintf(buf, 1024,logFormatMsg,##__VA_ARGS__);\
        logger.Log(buf);\
    }while(0)

#define LOG_ERROR(logFormatMsg,...)\
    do\
    {\
        Logger& logger = Logger::GetInstance();\
        logger.SetLogLevel(ERROR);\
        char buf[1024]={0};\
        snprintf(buf, 1024,logFormatMsg,##__VA_ARGS__);\
        logger.Log(buf);\
    }while(0)


#define LOG_FATAL(logFormatMsg,...)\
    do\
    {\
        Logger& logger = Logger::GetInstance();\
        logger.SetLogLevel(FATAL);\
        char buf[1024]={0};\
        snprintf(buf, 1024,logFormatMsg,##__VA_ARGS__);\
        logger.Log(buf);\
        exit(-1);\
    }while(0)


#ifdef MUDEBUG
#define LOG_DEBUG(logFormatMsg,...)\
    do\
    {\
        Logger& logger = Logger::GetInstance();\
        logger.SetLogLevel(DEBUG);\
        char buf[1024]={0};\
        snprintf(buf, 1024,logFormatMsg,##__VA_ARGS__)\
        logger.Log(buf);\
    }while(0)
#else
    #define LOG_DEBUG(logFromatMsg,...)
#endif

enum LogLevel{
    INFO,//普通消息 (普通流程消息)
    ERROR, //普通错误信息,不影响程序的正常的执行
    FATAL,//core信息, 毁灭性错信息,需要停止程序
    DEBUG,//调试信息
};

//日志类
class Logger: nocopyable
{
    public:
        //设置日志的级别
        void SetLogLevel(int level);

        //写日志
        void Log(std::string message);

        //获取Logger的单例实体
        static Logger& GetInstance();
    
    private:
        int logLevel_;

};