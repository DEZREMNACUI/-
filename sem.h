#ifndef _SEM_H /* 防止头文件被重复包含的保护宏开始 */
#define _SEM_H

#include <linux/sched.h> /* 包含内核调度相关的定义 */

/* 定义信号量表的最大长度，即系统中最多可以存在的信号量个数 */
#define SEMTABLE_LEN 20
/* 定义信号量名称的最大长度 */
#define SEM_NAME_LEN 20

/*
 * 定义信号量的数据结构
 * 该结构体用于表示一个信号量，包括信号量的名字、当前值以及等待该信号量的任务队列
 */
typedef struct semaphore
{
  char name[SEM_NAME_LEN];   /* 存储信号量的名称 */
  int value;                 /* 存储信号量的当前值 */
  struct task_struct *queue; /* 指向等待该信号量的任务队列的指针 */
} sem_t;

/*
 * 声明一个全局信号量表，它是一个sem_t类型的数组
 * 该数组存储了所有在系统中存在的信号量
 */
extern sem_t semtable[SEMTABLE_LEN];

#endif /* 结束防止头文件被重复包含的保护宏 */
