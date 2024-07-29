#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <unordered_map>
#include <initializer_list>
#include "log.h"
// 构造响应头的类
struct HeadMaker {
public:
    // 从 head_map 构造响应头
    std::string makeHeadFromMap(int code, std::string content_type, std::string content_len);
    void addEntry(const std::string& key, const std::string& value);
private:
    std::unordered_map<std::string, std::string> head_map;
    const std::unordered_map<int, const char*> stat_code = {
        {200, "200 OK"},
        {400, "400 Bad Request"},
        {401, "401 Unauthorized"},
        {403, "403 Forbidden"},
        {404, "404 Not Found"},
        {416, "Requested range not satisfiable"},
        {500, "500 Internal Server Error"},
        {503, "503 Server Unavailable"}
    };
};


// 服务的类族
class AbstractService {
public:
    AbstractService(){}
    virtual ~AbstractService() {}
    virtual std::string getResult(std::initializer_list<std::string> params) = 0;
    HeadMaker headMaker;
};
// 用于处理错误的服务
class BadService : public AbstractService {
public:
    BadService() {}
    virtual ~BadService() {}
    virtual std::string getResult(std::initializer_list<std::string> params);
};
// 资源请求类服务
class UriService : public AbstractService {
public:
    UriService() {}
    virtual ~UriService() {}
    // params 中有两个参数，一个是 uri 地址，另一个是 内容类型
    virtual std::string getResult(std::initializer_list<std::string> params);
    const std::string PAGE_HOME_PATH = std::string(PROJECT_PATH) + "/src/pages";
};

// 工厂
class ServiceFactory {
public:
    static AbstractService* getService(std::string api);
};
#endif