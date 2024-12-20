#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    // 客户端socket文件描述符
    int sock = 0;
    // 服务器地址结构
    struct sockaddr_in serv_addr;
    // 用于存储服务器主机信息的结构体指针
    struct hostent *server;
    // 用于存储接收数据的缓冲区
    char buffer[BUFFER_SIZE] = {0};
    
    // 创建socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket创建失败");
        exit(EXIT_FAILURE);
    }
    
    // 通过gethostbyname()函数获取本地服务器(localhost)的主机信息
    // 返回的server指针包含了主机的IP地址等网络信息
    server = gethostbyname("localhost");
    if (server == NULL) {
        perror("找不到主机");
        exit(EXIT_FAILURE);
    }
    
    // 设置服务器地址结构体的地址族为IPv4
    serv_addr.sin_family = AF_INET;
    // 将服务器IP地址从hostent结构复制到地址结构体中
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    // 设置服务器端口号,使用htons转换为网络字节序
    serv_addr.sin_port = htons(PORT);
    
    // 使用connect()函数尝试连接到服务器
    // sock: 客户端socket文件描述符
    // (struct sockaddr *)&serv_addr: 转换为通用socket地址结构体指针
    // sizeof(serv_addr): 地址结构体的大小
    // 如果连接失败(返回值<0),则打印错误信息并退出程序
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("连接失败");
        exit(EXIT_FAILURE);
    }
    // 发送消息
    char *message = "你好，服务器！";
    // send()函数用于向服务器发送数据
    // sock: 客户端socket文件描述符
    // message: 要发送的消息字符串
    // strlen(message): 消息的长度
    // 0: 发送标志,0表示默认行为
    send(sock, message, strlen(message), 0);
    printf("消息已发送\n");
    
    // 使用read()函数从服务器socket读取响应数据
    // sock: 客户端socket文件描述符
    // buffer: 用于存储接收数据的缓冲区
    // BUFFER_SIZE: 缓冲区大小
    // 返回值: 成功时返回读取的字节数,失败时返回-1
    read(sock, buffer, BUFFER_SIZE);
    printf("服务器响应: %s\n", buffer);
    
    close(sock);
    return 0;
} 