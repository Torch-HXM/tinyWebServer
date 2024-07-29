#include "service.h"

// 构造响应头
std::string HeadMaker::makeHeadFromMap(int code, std::string content_type, std::string content_len)
{
    std::string head;
    head.append("HTTP/1.1");
    head.push_back(' ');
    head.append(stat_code.at(code));
    head.append("\r\n");
    head.append("Content-Type: ");
    head.append(content_type);
    head.append("\r\n");
    head.append("Content-Length:");
    head.append(content_len);
    head.append("\r\n");

    for (auto& p:head_map) {
        head.append(p.first);
        head.append(": ");
        head.append(p.second);
        head.append("\r\n");
    }
    
    head.append("\r\n");

    return head;
}

void HeadMaker::addEntry(const std::string& key, const std::string& value)
{
    if (head_map.count(key)) {
        LOG_WARNING("[HeadMaker::addEntry]: Can't add same key to head map, so we update the value. Key: %s", key);
    }
    head_map.emplace(std::make_pair(std::move(key), std::move(value)));
}

// 返回 请求错误 头部
std::string BadService::getResult(std::initializer_list<std::string> params)
{
    HeadMaker headMaker;
    return headMaker.makeHeadFromMap(416, "text/plain", "0");
}

// 获得 uri 结果
std::string UriService::getResult(std::initializer_list<std::string> params)
{
    // 检查 params 内参数数量的合法性
    if (params.size() != 1) {
        LOG_ERROR("[UriService::getResult]: the len of params must be 2.");
        // 设置错误的 head, 400 错误的请求语法
        return headMaker.makeHeadFromMap(400, "text/plain", "0");

    }
    std::string path = *params.begin();
    std::string content_type;
    std::string body;

    if (path=="/") {
        path = PAGE_HOME_PATH + "/index.html";
    }
    else {
        path = PAGE_HOME_PATH + path;
    }
    
    if(
        path.at(path.size()-3)=='.' && 
        path.at(path.size()-2)=='j' && 
        path.at(path.size()-1)=='s') 
        {content_type = "text/js";}
    else if(
        path.at(path.size()-4)=='.' && 
        path.at(path.size()-3)=='c' && 
        path.at(path.size()-2)=='s' &&
        path.at(path.size()-1)=='s') 
        {content_type = "text/css";}
    else if(
        path.at(path.size()-5)=='.' &&
        path.at(path.size()-4)=='h' &&
        path.at(path.size()-3)=='t' &&
        path.at(path.size()-2)=='m' &&
        path.at(path.size()-1)=='l') 
        {content_type = "text/html";}
    else if(
        path.at(path.size()-4)=='.' && 
        path.at(path.size()-3)=='i' && 
        path.at(path.size()-2)=='c' &&
        path.at(path.size()-1)=='o') 
        {content_type = "image/x-icon";}
    else{
        LOG_ERROR("[UriService::getResult]:Unkonwn request file type: %s", path.c_str());
        return headMaker.makeHeadFromMap(416, "text/plain", "0");
    }

    std::ifstream ifs(path, std::ios_base::in);
    if(ifs.is_open()){
        body.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
    }
    else{
        LOG_ERROR("[UriService::getResult]:file open failed.");
        // 设置错误的 head, 500 服务器错误
        return headMaker.makeHeadFromMap(500, "text/plain", "0");
    }
    
    return headMaker.makeHeadFromMap(200, content_type, std::to_string(body.size())) + body;
}

AbstractService* ServiceFactory::getService(std::string api)
{
    if (api=="bad") {                                           // 返回错误头
        return new BadService();
    }
    else if (api.size()>3 && api.substr(0,3) == "api") {        // 预留给其他服务
        return nullptr;
    }
    else {                                                      // 资源请求型服务
        return new UriService();
    }
}
