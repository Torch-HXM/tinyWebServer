1. 安装

    ```bash
    cd webbench-1.5 && make
    sudo make install
    ```
2. 注意

    只有服务端发送完数据后主动断开连接，才会被 webbench 统计。

3. 本地测试

    ```bash
    webbench -c 10500 -t 5 http://localhost:50000/
    ```

