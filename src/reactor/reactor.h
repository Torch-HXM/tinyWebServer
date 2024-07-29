#ifndef REACTOR_H
#define REACTOR_H

#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <fcntl.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <unordered_map>
#include <map>

#include "log.h"
#include "service.h"
#include "mysql/mysql.h"

namespace reactor{

static const unsigned int SERVER_PORT(50000);
static const unsigned int MAX_WAIT_LISTEN(10000);
static const unsigned int MAX_EPOLL_LISTEN(10000);

static const char MYSQL_HOST[]        = "127.0.0.1";
static const char MYSQL_USERNAME[]    = "debian-sys-maint";
static const char MYSQL_PASSWORD[]    = "XIsPoeyd0dYWg4R5";
static const char MYSQL_DATABASE[]    = "user";
static const char MYSQL_TABLE_LOGIN[] = "main";

static bool stop = false;

// 一些基础的数据结构

template<class T>
struct ProducerConsumerUnit{
    std::queue<T> que;
    std::condition_variable cond;
    std::mutex mut;
    void addItem(T item)
    {
        {
            std::unique_lock<std::mutex> lock(mut);
            que.emplace(std::move(item));
            cond.notify_all();
        }
    }

    void getItem(T& des)
    {
        std::unique_lock<std::mutex> lock(mut);
        while(que.empty()&&!stop){
            cond.wait(lock);
        }
        if(stop) {
            lock.unlock();
            return;
        }

        des = que.front();

        que.pop();
        lock.unlock();
    }
};

template<class T>
struct SocketUnit{
    int fd;
    T data;

    SocketUnit(int fd, T&& data):fd(fd), data(std::move(data)){}
    SocketUnit():fd(-1){}
    SocketUnit(const SocketUnit<T>& su):fd(su.fd), data(std::move(su.data)){}

    void operator=(const SocketUnit<T>& su)
    {
        fd = su.fd;
        data = std::move(su.data);
        counter = counter;
    }
};

class Reactor{
public:
    static Reactor* getInstance(){
        static Reactor reactor;
        return &reactor;
    }
    void play();
private:
    Reactor(){};
    Reactor& operator=(const Reactor&)=delete;
    Reactor(const Reactor&&)=delete;
    ~Reactor();

    bool initTCPSocket();
    bool initEpoll();
    bool initMysql();
    bool initThread();

    void acceptThd();
    void receiveThd();
    void parserThd();
    void serviceThd();
    void sendThd();
    void closeThd();

    void alarmAddFd(int fd);
    void alarmRemoveFd(int fd);
    void alarmTriggerFunc();
    std::unordered_map<int, time_t> alarm_map;
    std::mutex alarm_mut;

    ProducerConsumerUnit<int> fp_accept;
    ProducerConsumerUnit<int> fp_receive;
    ProducerConsumerUnit<SocketUnit<std::string>> fp_parser;
    ProducerConsumerUnit<SocketUnit<std::unordered_map<std::string, std::string>>> fp_service;
    std::vector<SocketUnit<std::string>> result_vec;
    ProducerConsumerUnit<int> fp_send;
    ProducerConsumerUnit<int> fp_close;
    std::unordered_map<int, std::string> result_map;
    std::mutex result_map_mut;

    std::vector<std::thread*> thd_ptr_vec;
    int listen_fd;
    int epoll_fd;
    MYSQL* mysql;
    std::mutex mut_map_send;
    std::map<int, std::string> map_send;
};
}
#endif