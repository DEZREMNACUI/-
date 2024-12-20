#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    // 服务器socket文件描述符和新连接socket
    int server_fd, new_socket;
    // 服务器地址结构和客户端地址结构
    struct sockaddr_in server_addr, client_addr;
    // 客户端地址长度
    int addr_len = sizeof(client_addr);
    // 用于存储接收数据的缓冲区
    char buffer[BUFFER_SIZE] = {0};
    
    // 创建socket
    // AF_INET: 使用IPv4协议
    // SOCK_STREAM: 使用TCP协议
    // 0: 使用默认协议
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        // 如果socket创建失败,打印错误信息并退出程序
        perror("socket创建失败");
        exit(EXIT_FAILURE);
    }
    
    // 设置服务器地址结构
    // 设置地址族为IPv4
    server_addr.sin_family = AF_INET;
    // 设置IP地址为INADDR_ANY,表示接受来自任何IP地址的连接
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // 设置端口号,使用htons转换为网络字节序
    server_addr.sin_port = htons(PORT);
    
    // 将socket绑定到指定的IP地址和端口
    // bind()函数将server_fd(socket文件描述符)与server_addr(服务器地址结构)绑定
    // 如果返回值小于0表示绑定失败，成功返回0
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind失败");  // 打印错误信息
        exit(EXIT_FAILURE);  // 退出程序
    }
    
    // 开始监听socket连接
    // listen()函数将socket设置为监听模式
    // 第一个参数server_fd是要监听的socket文件描述符
    // 第二个参数3是等待连接队列的最大长度
    // 如果返回值小于0表示设置监听失败
    if (listen(server_fd, 3) < 0) {
        perror("listen失败");  // 打印错误信息
        exit(EXIT_FAILURE);    // 退出程序
    }
    
    printf("服务端正在监听端口 %d...\n", PORT);
    
    // accept()函数用于接受客户端的连接请求
    // accept 会阻塞等待，直到有客户端连接
    // server_fd: 服务器socket文件描述符
    // client_addr: 用于存储客户端地址信息的结构体
    // addr_len: 客户端地址结构的长度
    // 返回值: 成功时返回新的socket文件描述符,失败时返回-1
    if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addr_len)) < 0) {
        perror("accept失败");  // 如果accept失败,打印错误信息
        exit(EXIT_FAILURE);    // 退出程序
    }
    
    // inet_ntoa()函数将网络字节序的IP地址转换为点分十进制字符串格式
    // client_addr.sin_addr包含了客户端的IP地址
    // 打印连接成功的消息和客户端的IP地址
    printf("客户端连接成功，IP地址: %s\n", inet_ntoa(client_addr.sin_addr));
    
    // 使用read()函数从客户端socket读取数据
    // new_socket: 客户端连接的socket文件描述符
    // buffer: 用于存储接收数据的缓冲区
    // BUFFER_SIZE: 缓冲区大小
    // 返回值: 成功时返回读取的字节数,失败时返回-1
    read(new_socket, buffer, BUFFER_SIZE);
    
    // 打印从客户端接收到的消息
    // buffer中存储了客户端发送的数据
    printf("收到客户端消息: %s\n", buffer);

    // 发送响应
    // 创建一个响应字符串
    char *response = "服务器已收到消息";
    // send()函数将响应字符串发送给客户端
    // new_socket: 客户端连接的socket文件描述符
    // response: 响应字符串
    // strlen(response): 响应字符串的长度
    // 0: 发送标志,0表示默认行为
    send(new_socket, response, strlen(response), 0);
    
    // 关闭客户端连接的socket
    close(new_socket);
    // 关闭服务器监听的socket
    close(server_fd);
    return 0;
} 