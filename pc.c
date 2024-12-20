#include <unistd.h>    // 提供基本的 POSIX 系统 API
#include <sys/types.h> // 包含各种数据类型的定义，如 pid_t
#include <stdio.h>     // 标准输入输出函数
#include <stdlib.h>    // 提供通用工具函数，如 malloc 和 exit
#include <fcntl.h>     // 文件控制选项，例如 O_CREAT
#include <sys/stat.h>  // 文件状态标志和权限设置
#include <semaphore.h> // 信号量相关的函数和类型定义
#include <sys/types.h> // 再次包含以确保兼容性（实际上可能不需要）
#include <sys/wait.h>  // 提供 wait 和 waitpid 函数

#define M 530      // 定义生产者将生产的数字总数
#define N 5        // 定义消费者进程的数量
#define BUFSIZE 10 // 定义共享缓冲区的大小
//文件作为共享存储
// 生产者进程和消费者进程共享一个文件，文件中存储了缓冲区和buf_out
// 生产者进程将数据写入文件，消费者进程从文件中读取数据
// 生产者进程和消费者进程通过信号量协调对文件的访问
// 生产者进程和消费者进程通过文件指针协调对文件的访问

int main()
{
  sem_t *empty, *full, *mutex; // 声明三个信号量：empty（空槽位），full（满槽位），mutex（互斥锁）
  int fd;                      // 文件描述符，用于访问共享缓冲区
  int i, j, k, child;          // 循环变量
  int data;                    // 生产的数据
  pid_t pid;                   // 子进程ID
  int buf_out = 0;             // 记录上次从缓冲区读取的位置
  int buf_in = 0;              // 记录上次写入缓冲区的位置

  // 创建信号量
  empty = sem_open("empty", O_CREAT | O_EXCL, 0644, BUFSIZE); // 创建或打开名为 "empty" 的信号量，初始值为缓冲区大小
  full = sem_open("full", O_CREAT | O_EXCL, 0644, 0);         // 创建或打开名为 "full" 的信号量，初始值为0
  mutex = sem_open("mutex", O_CREAT | O_EXCL, 0644, 1);       // 创建或打开名为 "mutex" 的信号量，初始值为1

  // 在文件中存储 buf_out（因为生产者只有一个进程，所以 buf_in 不用存在文件中）
  fd = open("buffer.txt", O_CREAT | O_TRUNC | O_RDWR, 0666); // 打开或创建一个文件用于存储缓冲区和 buf_out
  // lseek 函数用于设置文件指针的位置,可以从当前位置移动指针
  lseek(fd, BUFSIZE * sizeof(int), SEEK_SET); // 将文件指针移动到缓冲区末尾，为存储 buf_out 变量预留空间。缓冲区占用 BUFSIZE * sizeof(int) 字节，buf_out 将存储在这之后
  write(fd, (char *)&buf_out, sizeof(int));   // 将 buf_out 存入文件中，以便子进程之间通信

  // 创建生产者进程
  if ((pid = fork()) == 0)
  {
    printf("I'm producer. pid = %d\n", getpid()); // 输出生产者的 PID
    for (i = 0; i < M; i++)                       // 按照 M 个数据循环生产
    {
      sem_wait(empty); // 等待有空闲槽位 (empty--)
      sem_wait(mutex); // 获取互斥锁 (mutex--)

      // 写入数据到缓冲区
      lseek(fd, buf_in * sizeof(int), SEEK_SET); // 移动文件指针到下一个空槽位
      write(fd, (char *)&i, sizeof(int));        // 写入当前数据
      buf_in = (buf_in + 1) % BUFSIZE;           // 更新写入位置

      sem_post(mutex); // 释放互斥锁 (mutex++)
      sem_post(full);  // 增加满槽位计数，可能唤醒消费者 (full++)
    }
    printf("producer end.\n");
    fflush(stdout); // 确保所有输出被立刻刷新
    return 0;
  }
  else if (pid < 0)
  {
    perror("Fail to fork!\n"); // 如果 fork 失败则输出错误信息
    return -1;
  }

  // 创建 N 个消费者进程
  for (j = 0; j < N; j++)
  {
    if ((pid = fork()) == 0)
    {
      for (k = 0; k < M / N; k++) // 每个消费者进程消费 M/N 个数据
      {
        sem_wait(full);  // 等待有满槽位 (full--)
        sem_wait(mutex); // 获取互斥锁 (mutex--)

        // 多个消费者需要协调读取位置
        lseek(fd, BUFSIZE * sizeof(int), SEEK_SET); // 去共享位置读取buf_out
        read(fd, &buf_out, sizeof(int));            // 获取当前应该读哪里

        lseek(fd, buf_out * sizeof(int), SEEK_SET); // 去读实际数据
        read(fd, &data, sizeof(int));               // 读取数据

        buf_out = (buf_out + 1) % BUFSIZE;          // 更新位置
        lseek(fd, BUFSIZE * sizeof(int), SEEK_SET); // 回到共享位置
        write(fd, &buf_out, sizeof(int));           // 保存新位置供其他消费者使用

        sem_post(mutex); // 释放互斥锁 (mutex++)
        sem_post(empty); // 增加空槽位计数，可能唤醒生产者 (empty++)
        // 输出消费的数据
        printf("%d:  %d\n", getpid(), data);
        fflush(stdout);
      }
      printf("child-%d: pid = %d end.\n", j, getpid());
      return 0;
    }
    else if (pid < 0)
    {
      perror("Fail to fork!\n"); // 如果 fork 失败则输出错误信息
      return -1;
    }
  }

  // 父进程等待所有子进程结束
  child = N + 1;
  while (child--)
    wait(NULL);

  // 释放信号量资源
  sem_unlink("full");
  sem_unlink("empty");
  sem_unlink("mutex");

  // 关闭文件
  close(fd);
  return 0;
}