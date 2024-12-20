#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// 线程函数
void *thread_function(void *arg)
{
  int thread_id = *(int *)arg; // 转换参数类型

  for (int i = 0; i < 3; i++)
  {
    printf("线程 %d 正在运行，打印第 %d 次\n", thread_id, i + 1);
    sleep(1); // 暂停1秒
  }

  return NULL;
}

int main()
{
  pthread_t thread1, thread2; // 定义线程变量
  int id1 = 1, id2 = 2;       // 传递给线程的参数

  // 创建第一个线程
  //pthread_create(线程变量, 线程属性, 线程函数, 线程参数);
  if (pthread_create(&thread1, NULL, thread_function, &id1) != 0)
  {
    printf("创建线程1失败\n");
    return 1;
  }

  // 创建第二个线程
  if (pthread_create(&thread2, NULL, thread_function, &id2) != 0)
  {
    printf("创建线程2失败\n");
    return 1;
  }

  // 等待两个线程结束
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  printf("所有线程已结束\n");
  return 0;
}
