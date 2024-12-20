#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main()
{
  int server_fd;
  struct sockaddr_in server_addr, client_addr;
  char buffer[BUFFER_SIZE];
  socklen_t client_len = sizeof(client_addr);

  // 1. 创建 UDP socket
  if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket 创建失败");
    exit(EXIT_FAILURE);
  }

  // 2. 设置服务器地址结构
  memset(&server_addr, 0, sizeof(server_addr));
  // 设置地址族为IPv4
  server_addr.sin_family = AF_INET;
  // 设置IP地址为INADDR_ANY,表示接受来自任何IP地址的数据
  server_addr.sin_addr.s_addr = INADDR_ANY;
  // 设置端口号,使用htons转换为网络字节序
  server_addr.sin_port = htons(PORT);

  // 3. 绑定 socket
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("bind 失败");
    exit(EXIT_FAILURE);
  }

  printf("UDP 服务器已启动，监听端口 %d...\n", PORT);

  // 4. 接收和发送数据
  while (1)
  {
    // 接收数据
    int bytes_received = recvfrom(server_fd, buffer, BUFFER_SIZE, 0,
                                  (struct sockaddr *)&client_addr, &client_len);

    if (bytes_received < 0)
    {
      perror("recvfrom 失败");
      continue;
    }

    // 打印客户端信息和数据
    buffer[bytes_received] = '\0';
    printf("收到来自 %s:%d 的消息: %s\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port),
           buffer);

    // 发送响应
    const char *response = "服务器已收到消息";
    sendto(server_fd, response, strlen(response), 0,
           (struct sockaddr *)&client_addr, client_len);
  }

  close(server_fd);
  return 0;
}
