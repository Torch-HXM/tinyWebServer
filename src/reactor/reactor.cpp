#include "reactor.h"

reactor::Reactor::~Reactor()
{
    // 通知所有循环，服务器已停止
    stop = true;
    fp_accept.cond.notify_all();
    fp_receive.cond.notify_all();
    fp_send.cond.notify_all();
    fp_close.cond.notify_all();
    fp_parser.cond.notify_all();
    fp_service.cond.notify_all();
    // 关闭线程
    for(auto& t:thd_ptr_vec){
        t->join();
        delete t;
        t = nullptr;
    }
    // 关闭监听套接字
    if(listen_fd>=0){
        close(listen_fd);
    }

    LOG_INFO_STD<<"Reactor shut down."<<LOG_END_STD;
}

//
// 初始化
//

bool reactor::Reactor::initTCPSocket()
{
    // 建立监听套接字
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);                        // 创建套接字文件，获得文件描述符
    if(listen_fd < 0){
        LOG_ERROR("[Reactor::initTCPSocket]:Create listen-socket failed.");
		return false;
	}
    LOG_INFO("[Reactor::initTCPSocket]: listen port [%d].", listen_fd);
    // 让套接字非阻塞
    int listen_fd_flag = fcntl(listen_fd,F_GETFL,0);
    if(listen_fd_flag<0)
    {
        LOG_ERROR("[Reactor::initTCPSocket]:fcntl F_GETFL fail.");
        close(listen_fd);
        return false;
    }
    else if(fcntl(listen_fd, F_SETFL, listen_fd_flag | O_NONBLOCK) < 0){
            LOG_ERROR("[Reactor::initTCPSocket]:fcntl F_SETFL fail.");
            close(listen_fd);
            return false;
    }
    else{}
    // 绑定套接字到端口
    struct sockaddr_in server_socket_addr;
    memset(&server_socket_addr, 0, sizeof(server_socket_addr));         // 创建tcp套接字属性结构体
	server_socket_addr.sin_family = AF_INET;                            // IPv4协议
	server_socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);	            // INADDR_ANY转换过来就是0.0.0.0，表示在服务器全网卡上监听
	server_socket_addr.sin_port = htons(SERVER_PORT);                   // 将端口转换为网络形式

    // 绑定套接字与套接字属性
    if(bind(listen_fd,(struct sockaddr*)&server_socket_addr, sizeof(server_socket_addr)) < 0){ 
        LOG_ERROR("[Reactor::initTCPSocket]:Bind listen-socket and listen-port failed. %s", strerror(errno));
        return false;
    }

    // 让套接字处于被动监听模式
    // 设置可同时排队的最大客户端数
    if(listen(listen_fd, MAX_WAIT_LISTEN) < 0){     
        LOG_ERROR("[Reactor::initTCPSocket]:Set max number of waiting clients for listen-socket failed. %s", strerror(errno));
        return false;
    }

    LOG_INFO("[Reactor::initTCPSocket]: Init successfully.");
    LOG_INFO_STD<<"[Reactor::initTCPSocket]: Init successfully."<<LOG_END_STD;
    return true;
}

bool reactor::Reactor::initEpoll()
{
    // listen socket 的阻塞与非阻塞并无所谓
    // 创建 epoll 实例
    epoll_fd = epoll_create(MAX_EPOLL_LISTEN);
    if(epoll_fd < 0){
        LOG_ERROR("[Reactor::initEpoll]:Create epoll failed. %s", strerror(errno));
        return false;
    }
    // 初始化 epoll
    struct epoll_event ev;
    ev.events = EPOLLIN|EPOLLET;        // 设置上升沿触发，异步通知
	ev.data.fd = listen_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0){     // 向epoll的监听队列中加入监听套接字
        LOG_ERROR("[Reactor::initEpoll]:Add listen-socket to epoll failed. %s", strerror(errno));
        return false;
    }

    LOG_INFO("[Reactor::initEpoll]: Init successfully.");
    LOG_INFO_STD<<"[Reactor::initEpoll]: Init successfully."<<LOG_END_STD;
    return true;
}

bool reactor::Reactor::initMysql()
{
    if((mysql = mysql_init(mysql))==NULL){
        LOG_ERROR("[Reactor::initMysql]:mysql init failed. %s", mysql_error(mysql));
        return false;
    }
    mysql = mysql_real_connect(
                mysql,
                MYSQL_HOST,
                MYSQL_USERNAME,
                MYSQL_PASSWORD,
                MYSQL_DATABASE,
                0,
                NULL,
                0
            );
    if(mysql==NULL){
        LOG_ERROR("[Reactor::initMysql]:mysql connect failed. %s", mysql_error(mysql));
        return false;
    }

    LOG_INFO("[Reactor::initMysql]: Init successfully.");
    LOG_INFO_STD<<"[Reactor::initMysql]: Init successfully."<<LOG_END_STD;
    return true;
}

bool reactor::Reactor::initThread()
{
    // 创建接收链接线程，读取线程
    thd_ptr_vec.emplace_back(new std::thread(&reactor::Reactor::closeThd, this));
    thd_ptr_vec.emplace_back(new std::thread(&reactor::Reactor::acceptThd, this));
    thd_ptr_vec.emplace_back(new std::thread(&reactor::Reactor::receiveThd, this));
    thd_ptr_vec.emplace_back(new std::thread(&reactor::Reactor::parserThd, this));
    thd_ptr_vec.emplace_back(new std::thread(&reactor::Reactor::serviceThd, this));
    thd_ptr_vec.emplace_back(new std::thread(&reactor::Reactor::sendThd, this));
    

    LOG_INFO("[Reactor::initThread]: Init successfully.");
    LOG_INFO_STD<<"[Reactor::initThread]: Init successfully."<<LOG_END_STD;
    return true;
}

// 向计时器中注册端口
void reactor::Reactor::alarmAddFd(int fd)
{
    {
        std::unique_lock<std::mutex> lock(alarm_mut);
        time_t now_time = time(NULL);
        if (alarm_map.count(fd)) {
            LOG_WARNING("[reactor::Reactor::alarmAddFd]:fd %d already in alarm map.", fd);
            alarm_map.at(fd) = now_time;
        } else {
            alarm_map.emplace(fd, now_time);
        }
    }
}
// 从计时器中移除端口
void reactor::Reactor::alarmRemoveFd(int fd)
{
    {
        std::unique_lock<std::mutex> lock(alarm_mut);
        if (alarm_map.count(fd)) {
            alarm_map.erase(fd);
        } else {
            LOG_ERROR("[reactor::Reactor::alarmRemoveFd]:Can't find fd %d in alarm map.", fd);
        }
    }
}
// 计时器的回调函数，负责遍历已注册的fd，并关闭超时fd
void reactor::Reactor::alarmTriggerFunc()
{

}

// 为了避免无意义的循环浪费 cpu 资源，让 accept 在 epoll 的指导下工作。
// 监听 listen_fd 被注册为非阻塞模式。
// 如果 accept 因没有需要建立的链接而失败，则 continue。
// 如果 accept 因为其他错误而建立连接失败，则 continue。
// accept 成功后，将 fd 设为非阻塞。
// 若设为非阻塞失败，则关闭连接。
// 若设置非阻塞成功则向 epoll 中注册该 fd 的监听事件。
// 若注册失败，则关闭链接。
void reactor::Reactor::acceptThd()
{
    int nothing;
    while(!stop){
        int accept_fd = -1;
        fp_accept.getItem(nothing);
        if (stop) break;
        if((accept_fd = accept(listen_fd, NULL, NULL))<0){  // 表示并没有需要建立的链接，
            if(errno!=(EWOULDBLOCK|EAGAIN)){
                LOG_WARNING("[Reactor::acceptThd]:Connection established failed.:%s", strerror(errno));
            }
            continue;
        }
        else{
            // 将 fd 设为非阻塞
            int accept_fd_flag = fcntl(accept_fd,F_GETFL,0);
            if(accept_fd_flag<0)
            {
                LOG_ERROR("[Reactor::acceptThd]:fcntl F_GETFL fail.");
                close(accept_fd); 
            }
            else{
                if(fcntl(accept_fd, F_SETFL, accept_fd_flag | O_NONBLOCK) < 0){
                    LOG_ERROR("[Reactor::acceptThd]:fcntl F_SETFL fail.");
                    close(accept_fd);   
                }
                else{
                    // 向 epoll 中注册
                    struct epoll_event ev;
                    // 关于 EPOLLRDHUP
                    // Stream  socket  peer  closed  connection, or shut down writing half of connection.  (This flag is espe‐cially useful for writing simple code to detect peer shutdown when using Edge Triggered monitoring.)
                    ev.events = EPOLLIN|EPOLLET|EPOLLRDHUP;    // 监听套接字的可读事件，以边沿模式监听，注册EPOLLRDHUP用于监听对端FIN包
                    ev.data.fd = accept_fd;
                    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev)<0){
                        LOG_ERROR("[Reactor::acceptThd]:Fd:[%d] epoll_ctl Error:%s", ev.data.fd, strerror(errno));
                        close(accept_fd);
                        continue;
                    }
                    LOG_INFO("[Reactor::acceptThd]:Fd:[%d] accepted.", ev.data.fd);
                }
            }
        }
    }
}

// 循环读取缓冲区，知道读取完毕或出现读取异常。
// 因为 fd 为非阻塞，所以读取完毕后 read 函数应当返回 -1，且错误码为 EAGAIN。
// 如果 read 函数返回负数但错误码不是 EAGIN 说明发生了错误，则关闭 fd。
// 若 read 函数返回 0 则代表客户端关闭了链接，此时关闭 fd。
// 成功接收后，将接收到的内容与 fd 一起加入到 fd_praser 中等待解析。
void reactor::Reactor::receiveThd()
{
    
    while(!stop){
        int fd;
        fp_receive.getItem(fd);                                 // 获得待读取的套接字
        if (stop) break;
        // 接收数据
        std::string client_msg("");          // 用于储存读取到的消息, 需要在出错或者解析数据后释放
        ssize_t len = 0;
        while(!stop){
            char buf[1024] = {'\0'};
            len = read(fd, buf, 1024);
            if(len>0){
                client_msg.append(buf);
            }
            else{
                break;
            }
        }
        if(stop) break;
        // 判断端口状态
        if(len<0 && errno!=EAGAIN){                             // fd 为非阻塞，数据读取完毕返回 -1， errno为EAGAIN
            LOG_ERROR("[reactor::Reactor::receiveThd]:Read error. %s", strerror(errno));
            fp_close.addItem(fd);
            continue;
        }
        else if(len==0){                                       // 读取到0意味着对端关闭
            LOG_WARNING("[reactor::Reactor::receiveThd]:Client %d closed.", fd);                       
            fp_close.addItem(fd);                              // 加入关闭队列
            continue;
        }
        else{
            // 成功接收, 加入解析队列
            LOG_INFO("[reactor::Reactor::receiveThd]:Successfully receive fd %d", fd);
            // LOG_INFO("[reactor::Reactor::receiveThd]:Successfully receive fd %d : %s", fd, client_msg.c_str());
            fp_parser.addItem(SocketUnit<std::string>(fd, std::move(client_msg)));
        }
    }
}

// 如果解析报文时出错，则令 service 字段为 "bad"，交给 serviceThd,让它得到错误头结果。
void reactor::Reactor::parserThd()
{
    while(!stop){
        SocketUnit<std::string> item;
        fp_parser.getItem(item);
        if (stop) break;
        // 开始解析
        // 将 html 报文分行
        std::unordered_map<std::string, std::string> html_map;
        std::vector<std::string> lines;
        lines.push_back(std::string(""));
        for(char c:item.data)
        {
            if(c=='\r') continue;
            else if(c=='\n')
            {
                if(!lines.back().empty()) lines.push_back(std::string(""));
            }
            else lines.back().push_back(c);
        }

        // 第一行包含 请求方法、请求资源名、html 版本
        int pos_counter = 0;
        std::string method;
        std::string source_name;
        std::string version;
        for(char c:lines[0]){
            if(c==' ') ++pos_counter;
            else{
                if(pos_counter==0) method.push_back(c);
                else if(pos_counter==1) source_name.push_back(c);
                else if(pos_counter==2) version.push_back(c);
                else{
                    LOG_WARNING("[reactor::Reactor::parserThd]: Illegal request message.");
                    // 出现非法请求时，在 service 中标记
                    source_name = "bad";
                    break;
                }
            }
        }

        if(html_map.count("method")==0)
        {
            html_map.emplace(std::make_pair("method", method));
        }
        else
        {
            LOG_WARNING("[reactor::Reactor::parserThd]:Multi method. Use the newest.");
            html_map.at("method") = method;
        }

        if(html_map.count("service")==0)
        {
            html_map.emplace(std::make_pair("service", source_name));
        }
        else
        {
            LOG_WARNING("[reactor::Reactor::parserThd]:Multi service. Use the newest.");
            html_map.at("service") = source_name;
        } 

        if(html_map.count("version")==0)
        {
            html_map.emplace(std::make_pair("version", version));
        } 
        else
        {
            LOG_WARNING("[reactor::Reactor::parserThd]:Multi version. Use the newest.");
            html_map.at("version") = version;
        }   

        if (html_map.at("service") == "bad" || lines.empty()) {         // 如果标记为 bad，或者 lines 中没有报文，不用继续解析，直接发送给 service ，让其返回错误头
            html_map.at("service") = "bad";
            fp_service.addItem(SocketUnit<std::unordered_map<std::string, std::string>>(item.fd, std::move(html_map)));
            continue;
        }

        lines.erase(lines.begin());
        //// 其余的行都是键值对
        for(std::string& line : lines)
        {
            std::string key;
            std::string value;
            bool splite_flag = false;
            for(char c:line)
            {
                if (c==':') splite_flag = true;
                else if (!splite_flag) key.push_back(c);
                else value.push_back(c);
            }
            if (value.size()) value.erase(value.begin());       // 去空格
            else continue;                                      // 如果值为空，则不记录该 entry

            if(html_map.count(key)==0)
            {
                html_map.emplace(std::move(key), std::move(value));
            }
            else    // 如果出现重复的键值，则报警告，并使用最新键值
            {
                LOG_WARNING("[reactor::Reactor::parserThd]: key value conflate. Use the newest. Key: %s", key.c_str());
                html_map.at(key) = value;
            } 
        }
        // 成功解析，加入服务队列
        fp_service.addItem(SocketUnit<std::unordered_map<std::string, std::string>>(item.fd, std::move(html_map)));
    }
}

// 如果是未知的服务类型，则返回错误头
void reactor::Reactor::serviceThd()
{
    while(!stop){
        SocketUnit<std::unordered_map<std::string, std::string>> item;
        fp_service.getItem(item);
        if (stop) break;

        AbstractService* service = ServiceFactory::getService(item.data.at("service"));
        std::string result;
        if (service == nullptr) {
            LOG_ERROR("[Reactor::serviceThd]:No service found.");
            HeadMaker headMaker;
            result = headMaker.makeHeadFromMap(500, "text/plain", "0");
            continue;
        }
        result = service->getResult({item.data.at("service")});
        delete service;
        service = nullptr;

        // 将结果加入待发送队列
        {
            std::unique_lock<std::mutex> lock(result_map_mut);
            result_map.emplace(item.fd, std::move(result));
        }
    
        // 监听 fd 的可发送事件
        epoll_event ev;
        ev.data.fd = item.fd;
        ev.events  = EPOLLOUT|EPOLLET;

        if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ev.data.fd, &ev)<0){
            LOG_ERROR("[Reactor::serviceThd]:Fd:[%d] epoll_ctl Error:%s", ev.data.fd, strerror(errno));
            continue;
        }
    }
}

void reactor::Reactor::sendThd()
{
    while(!stop){
        std::string result;
        // 获得对应结果
        int fd;
        fp_send.getItem(fd);
        if (stop) break;

        {
            std::unique_lock<std::mutex> lock(result_map_mut);
            // 在 result_map 中查找 fd
            if(result_map.count(fd)==0) {   // 如果没有找到则代表发生了错误，需要返回一个错误头
                LOG_ERROR("[Reactor::sendThd]: Can't find result related to fd %d", fd);
                HeadMaker headMaker;
                result = headMaker.makeHeadFromMap(500, "text/plain", "0");
            }
            else {
                result = result_map.at(fd);
                result_map.erase(fd);
            }
        }
        // 发送数据
        int write_bytes = write(fd, result.c_str(), result.size());
        if(write_bytes<0){
            LOG_ERROR("[Reactor::sendThd]:Write error. %s", strerror(errno));
        }
        else {
            LOG_INFO("[Reactor::sendThd]:send successfully. fd: %d", fd);
        }
        fp_close.addItem(fd);
    }
}

void reactor::Reactor::closeThd()
{
    while(!stop)
    {
        int fd;
        fp_close.getItem(fd);
        if (stop) break;

        epoll_event ev;
        ev.data.fd = fd;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev)<0){
            LOG_ERROR("[Reactor::closeThd]:Fd:[%d] epoll_ctl Error:%s", fd, strerror(errno));
        }
        else{
            close(fd);
            LOG_INFO("[Reactor::closeThd]:Close client fd:[%d].", fd);
        }
    }
}

//
// 主循环
//

void reactor::Reactor::play()
{
    if(initTCPSocket()&&initEpoll()&&initMysql()&&initThread()){
        LOG_INFO("[Reactor::play]:Init all Success.");
        LOG_INFO_STD<<"[Reactor::play]:Init all Success."<<LOG_END_STD;
        // ctrl+c 信号捕捉
        auto func_sig = [](int sig){
            LOG_WARNING("[Reactor::mainReactorFunc]:Get ctrl+c signal. Main loop terminal.");
            LOG_WARNING_STD<<"\nGet ctrl+c signal. Main loop terminal."<<LOG_END_STD;
            stop = true;
            signal(sig, SIG_DFL);
        };
        signal(SIGINT, func_sig);

        // 主循环
        struct epoll_event events[MAX_EPOLL_LISTEN];
        while(!stop){
            int active_port_num = epoll_wait(epoll_fd, events, MAX_EPOLL_LISTEN, -1);
            if(active_port_num < 0){                // 发生错误
                LOG_ERROR("[Reactor::play]:Epoll-wait-fault. Error:%s", strerror(errno));
            }
            else if(active_port_num==0){            // 发生超时
                LOG_WARNING("[Reactor::play]:Thread Reactor::mainReactorFunc happends to timeout. Continue.");
            }
            else{
                for(int i=0;i<active_port_num;i++){         // 鉴别端口事务的类型
                    // EPOLLERR 读关闭
                    // Error condition happened on the associated file descriptor.  This event is also reported for the  write
                    // end  of  a pipe when the read end has been closed.  epoll_wait(2) will always report for this event; it
                    // is not necessary to set it in events.

                    // EPOLLHUP 关闭
                    // Hang up happened on the associated file descriptor.  epoll_wait(2) will always wait for this event;  it
                    // is not necessary to set it in events.

                    // Note  that  when  reading from a channel such as a pipe or a stream socket, this event merely indicates
                    // that the peer closed its end of the channel.  Subsequent reads from the channel will return 0  (end  of
                    // file) only after all outstanding data in the channel has been consumed.
                    if(events[i].data.fd == listen_fd) // 建立连接
                    {
                        fp_accept.addItem(0);
                    }
                    else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))// 客户端读关闭
                    {   
                        LOG_ERROR("[Reactor::play]: fd %d occus EPOLLRDHUP | EPOLLHUP | EPOLLERR.", events[i].data.fd);
                        fp_close.addItem(events[i].data.fd);
                    }
                    else if(events[i].events & EPOLLIN)     // 接收数据
                    {    
                        fp_receive.addItem(events[i].data.fd);
                    }
                    else if(events[i].events & EPOLLOUT)    // 发送数据
                    {   
                        fp_send.addItem(events[i].data.fd);
                    }
                }
            }
        }
    }
}
