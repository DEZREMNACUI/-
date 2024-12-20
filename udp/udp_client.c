#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024

int main()
{
  int client_fd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  // 1. 创建 UDP socket
  if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket 创建失败");
    exit(EXIT_FAILURE);
  }

  // 2. 设置服务器地址结构
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  // 将IP地址从字符串转换为网络地址
  if (inet_aton(SERVER_IP, &server_addr.sin_addr) == 0)
  {
    perror("inet_aton 失败");
    exit(EXIT_FAILURE);
  }

  printf("UDP 客户端已启动，连接到 %s:%d\n", SERVER_IP, PORT);

  // 3. 发送和接收数据
  while (1)
  {
    printf("请输入消息: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // 删除换行符

    // 发送数据
    if (sendto(client_fd, buffer, strlen(buffer), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
      perror("sendto 失败");
      continue;
    }

    // 接收响应
    socklen_t server_len = sizeof(server_addr);
    int bytes_received = recvfrom(client_fd, buffer, BUFFER_SIZE, 0,
                                  (struct sockaddr *)&server_addr, &server_len);

    if (bytes_received < 0)
    {
      perror("recvfrom 失败");
      continue;
    }

    // 显示服务器响应
    buffer[bytes_received] = '\0';
    printf("服务器响应: %s\n", buffer);

    // 输入 'quit' 退出
    if (strcmp(buffer, "quit") == 0)
    {
      break;
    }
  }

  close(client_fd);
  return 0;
}
