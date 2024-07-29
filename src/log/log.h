#ifndef LOG_H
#define LOG_H

#include <queue>
#include <string>
#include <mutex>
#include <vector>
#include <sys/time.h>
#include <condition_variable>
#include <fstream>
#include <thread>
#include <functional>
#include <stdarg.h>
#include <iostream>

#define LOG_INFO_STD    std::cout<<"\033[32m"
#define LOG_WARNING_STD std::cout<<"\033[33m"
#define LOG_ERROR_STD   std::cout<<"\033[31m"
#define LOG_END_STD     "\033[0m\n"

namespace logger{

static const std::string LOG_FILE("log.txt");
static const unsigned int MAX_QUEUE_SIZE(500);
// 优雅的懒汉单例
class Log{
public:
    static Log* getInstance(){
        static Log log_instance;
        return &log_instance;    
    }
    void addItem(const char* keyword, const char* format, ...);
    
private:
    // 让默认构造函数为私有，方便静态实例创建
    Log():stop(false){thd = new std::thread(&Log::writeFunc, this);};  
    // 禁用赋值构造函数和拷贝构造函数
    Log& operator=(const Log&) = delete;
    Log(const Log&) = delete;
    virtual ~Log(){
        stop = true;
        cond.notify_all();
        if(thd!=nullptr){
            thd->join();
            delete thd;
            thd = nullptr;
        }
        LOG_INFO_STD<<"Log shut down."<<LOG_END_STD;
    }
    
    // 将日志写入文件的主逻辑
    void writeFunc();

    std::queue<std::string> message_queue;  // 待处理的消息队列
    std::mutex mut;                         // 互斥锁
    std::condition_variable cond;           // 条件变量
    std::thread* thd;                       // 写文件的任务线程
    bool stop;                              // 线程结束标志
};
};
#ifdef LOGGER
#define LOG_INFO(format, ...)       logger::Log::getInstance()->addItem(" [INFO]:    ", format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...)    logger::Log::getInstance()->addItem(" [WARNING]: ", format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)      logger::Log::getInstance()->addItem(" [ERROR]:   ", format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)       
#define LOG_WARNING(format, ...)    
#define LOG_ERROR(format, ...)      
#endif
#endif