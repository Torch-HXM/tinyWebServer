#include "log.h"

void logger::Log::writeFunc(){
    // 打开日志文件
    std::ofstream ofs;
    ofs.open(LOG_FILE, std::ios::app);    // 文件不存在时创建文件，追加
    // 循环写入
    while(true){
        std::unique_lock<std::mutex> lock(mut);
        while(message_queue.empty()){
            if(!stop){
                cond.wait(lock);
            }
            else{
                lock.unlock();
                ofs.close();
                return;
            }
        }
        ofs<<message_queue.front();
        ofs.flush();
        message_queue.pop();
        cond.notify_one();
        lock.unlock();
    }
    ofs.close();

}

void logger::Log::addItem(const char* keyword, const char* format, ...){
#ifdef LOGGER
    char buf[5120];
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t timep = now.tv_sec;
    struct tm *p;

    time(&timep); //获取从1970至今过了多少秒，存入time_t类型的timep
    p = localtime(&timep);//用localtime将秒数转化为struct tm结构体
    int n = snprintf(buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s",
                    p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
                    p->tm_hour, p->tm_min, p->tm_sec, now.tv_usec, keyword);
    
    va_list valst;
    va_start(valst, format);
    int m = vsnprintf(buf + n, 5118, format, valst);
    buf[n + m] = '\n';
    buf[n + m + 1] = '\0';
    {
        std::unique_lock<std::mutex> lock(mut);
        message_queue.emplace(std::string(buf));
        cond.notify_one();
    }
#endif
}