#define   __LIBRARY__      /* 定义宏，用于指示这是一个库文件或者需要特殊处理的源文件 */
#include <unistd.h>  	   /* 包含标准的UNIX系统调用接口 */
#include <sys/types.h>     /* 包含系统类型定义，例如 size_t, ssize_t 等 */
#include <fcntl.h>         /* 包含文件控制选项和命令的定义 */
#include <stdio.h>         /* 包含标准输入输出函数的声明 */
#include <stdlib.h>		   /* 包含标准库函数的声明，如 malloc(), exit() 等 */
#include <linux/sem.h>     /* 包含Linux信号量相关的函数和数据结构定义 */
/* 定义生产者将生产的数字总数 */
#define M 530            
/* 定义消费者进程的数量 */
#define N 5              
/* 定义共享缓冲区的大小 */
#define BUFSIZE 10      

/* 声明系统调用接口 */
//1,2代表参数个数
_syscall2(sem_t*, sem_open, const char *, name, unsigned int, value);
_syscall1(int, sem_wait, sem_t*, sem);
_syscall1(int, sem_post, sem_t*, sem);
_syscall1(int, sem_unlink, const char *, name);

int main()
{
    /* 初始化三个信号量：empty（空槽位），full（满槽位），mutex（互斥锁）*/
    sem_t *empty, *full, *mutex;  
    /* 文件描述符，用于访问共享缓冲区 */
    int fd;                       
    /* 循环变量 */
    int i,j,k,child;
    /* 生产的数据 */
    int data;                    
    /* 子进程ID */
    pid_t pid;
    /* 记录上次从缓冲区读取的位置 */
    int buf_out = 0;             
    /* 记录上次写入缓冲区的位置 */
    int buf_in = 0;              

    /* 创建互斥锁信号量 */
    if((mutex = sem_open("mutex", 1)) == NULL)
    {
        /* 如果创建失败，则输出错误信息并退出 */
        perror("sem_open() error!\n");
        return -1;
    }
    /* 创建empty信号量，初始值为缓冲区大小 */
    if((empty = sem_open("empty", BUFSIZE)) == NULL)
    {
        /* 如果创建失败，则输出错误信息并退出 */
        perror("sem_open() error!\n");
        return -1;
    }
    /* 创建full信号量，初始值为0 */
    if((full = sem_open("full", 0)) == NULL)
    {
        /* 如果创建失败，则输出错误信息并退出 */
        perror("sem_open() error!\n");
        return -1;
    }

    /* 打开或创建一个文件用于存储缓冲区和buf_out */
    fd = open("buffer.txt", O_CREAT|O_TRUNC|O_RDWR, 0666); 
    /* 设置文件指针到缓冲区后的第一个位置 */
    lseek(fd, BUFSIZE*sizeof(int), SEEK_SET); 

    /* 将buf_out存入文件中，以便子进程之间通信 */
    write(fd, (char *)&buf_out, sizeof(int));  

    /* 创建生产者进程 */
    if((pid=fork())==0)
    {
        /* 生产者进程开始 */
        printf("I'm producer. pid = %d\n", getpid());
        /* 按照M个数据循环生产 */
        for(i = 0; i < M; i++)
        {
            /* 等待有空闲槽位 */
            sem_wait(empty);  /* empty-- */
            /* 获取互斥锁 */
            sem_wait(mutex);  /* mutex-- */

            /* 写入数据到缓冲区 */
            lseek(fd, buf_in*sizeof(int), SEEK_SET); 
            write(fd, (char *)&i, sizeof(int)); 
            /* 更新写入位置 */
            buf_in = (buf_in + 1) % BUFSIZE;

            /* 释放互斥锁 */
            sem_post(mutex);  /* mutex++ */
            /* 增加满槽位计数，可能唤醒消费者 */
            sem_post(full);   
        }
        printf("producer end.\n");
        fflush(stdout); /* 确保所有输出被立刻刷新 */
        return 0;
    }
    else if(pid < 0)
    {
        /* 如果fork失败则输出错误信息 */
        perror("Fail to fork!\n");
        return -1;
    }

    /* 创建N个消费者进程 */
    for(j = 0; j < N ; j++)
    {
        if((pid=fork())==0)
        {
            /* 每个消费者进程消费M/N个数据 */
            for(k = 0; k < M/N; k++)
            {
                /* 等待有满槽位 */
                sem_wait(full);  
                /* 获取互斥锁 */
                sem_wait(mutex);

                /* 从文件中读取上次读取位置 */
                lseek(fd, BUFSIZE*sizeof(int), SEEK_SET);
                read(fd, (char *)&buf_out, sizeof(int));
                /* 从上次读取位置继续读取数据 */
                lseek(fd, buf_out*sizeof(int), SEEK_SET);
                read(fd, (char *)&data, sizeof(int));
                /* 更新下次应读取的位置 */
                buf_out = (buf_out + 1) % BUFSIZE;
                /* 将新的读取位置写回文件 */
                lseek(fd, BUFSIZE*sizeof(int), SEEK_SET);
                write(fd, (char *)&buf_out, sizeof(int));

                /* 释放互斥锁 */
                sem_post(mutex);  
                /* 增加空槽位计数，可能唤醒生产者 */
                sem_post(empty);  
                /* 输出消费的数据 */
                printf("%d:  %d\n", getpid(), data);
                fflush(stdout);
            }
            printf("child-%d: pid = %d end.\n", j, getpid());
            return 0;
        }
        else if(pid<0)
        {
            /* 如果fork失败则输出错误信息 */
            perror("Fail to fork!\n");
            return -1;
        }
    }

    /* 父进程等待所有子进程结束 */
    child = N + 1;
    while(child--)
        wait(NULL);

    /* 释放信号量资源 */
    sem_unlink("full");
    sem_unlink("empty");
    sem_unlink("mutex");

    /* 关闭文件 */
    close(fd);
    return 0;
}