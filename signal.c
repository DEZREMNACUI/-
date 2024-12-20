#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

// 信号处理函数
void signal_handler(int signo)
{
  switch (signo)
  {
  case SIGINT:
    printf("\n收到 Ctrl+C (SIGINT)!\n");
    break;

  case SIGTERM:
    printf("\n收到终止信号 (SIGTERM)!\n");
    printf("正在清理资源...\n");
    sleep(1);
    printf("清理完成，程序退出\n");
    exit(0);
    break;

  case SIGUSR1:
    printf("\n收到用户自定义信号 (SIGUSR1)!\n");
    break;
  }
}

int main()
{
  // 注册信号处理函数
  signal(SIGINT, signal_handler);  // Ctrl+C
  signal(SIGTERM, signal_handler); // kill 默认信号
  signal(SIGUSR1, signal_handler); // 用户自定义信号

  printf("进程已启动，PID = %d\n", getpid());
  printf("测试方法：\n");
  printf("1. 按 Ctrl+C\n");
  printf("2. kill %d     (发送 SIGTERM)\n", getpid());
  printf("3. kill -USR1 %d (发送 SIGUSR1)\n", getpid());

  // 保持程序运行
  while (1)
  {
    sleep(1);
  }

  return 0;
}
