#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
  pid_t pid;
  int count = 0;

  printf("父进程开始执行，PID = %d\n", getpid());

  // 创建子进程
  pid = fork();

  if (pid < 0)
  {
    // fork 失败
    printf("创建子进程失败!\n");
    return -1;
  }
  else if (pid == 0)
  {
    // 子进程执行的代码
    count++;
    printf("我是子进程: PID = %d, 父进程 PID = %d, count = %d\n",
           getpid(), getppid(), count);
    sleep(1); // 让子进程睡眠1秒
  }
  else
  {
    // 父进程执行的代码
    count++;
    printf("我是父进程: PID = %d, 子进程 PID = %d, count = %d\n",
           getpid(), pid, count);

    // 等待所有子进程结束
    while (wait(NULL) > 0)
    {
      // 每次等待一个子进程结束
    }
    printf("所有子进程都已结束\n");
  }

  return 0;
}
