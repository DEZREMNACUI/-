#include <linux/sem.h>    // 包含信号量相关的函数和数据结构定义
#include <linux/sched.h>  // 包含进程调度相关的函数和数据结构定义
#include <unistd.h>       // 包含标准的UNIX系统调用接口
#include <asm/segment.h>  // 包含与段寄存器操作有关的函数（过时）
#include <linux/tty.h>    // 包含TTY设备相关的函数和数据结构定义
#include <linux/kernel.h> // 包含内核级别的宏定义和其他工具函数
#include <linux/fdreg.h>  // 包含FDC控制器寄存器的定义（过时）
#include <asm/system.h>   // 包含体系结构特定的系统调用和中断处理函数
#include <asm/io.h>       // 包含输入输出端口访问的函数
//#include <string.h>
typedef struct semaphore
{
    char name[SEM_NAME_LEN];   /* 存储信号量的名称 */
    int value;                 /* 存储信号量的当前值 */
    struct task_struct *queue; /* 指向等待该信号量的任务队列的指针 */
} sem_t;

extern sem_t semtable[SEMTABLE_LEN]; 
/* 
 * 已创建信号量表，全局变量，用于存储所有已创建的信号量。
 * 每个信号量包含一个名字、一个值和一个等待队列。
 */
sem_t semtable[SEMTABLE_LEN];  
/* 记录已创建信号量个数，用于追踪当前有多少个信号量已经被创建 */
int cnt = 0;                   

/* 创建一个信号量 */
sem_t *sys_sem_open(const char *name,unsigned int value)
{
    /* 内核态使用的信号量名称缓冲区，长度应该足够大以容纳用户提供的名称 */
    char kernelname[100];   
    /* 标记信号量是否已经存在 */
    int isExist = 0;
    /* 循环计数器 */
    int i=0;
    /* 用户空间信号量名称的长度 */
    int name_cnt=0;

    /* 
     * 计算用户空间提供的信号量名称长度。
     * 使用get_fs_byte函数逐字节从用户空间复制到内核空间。
     */
    while( get_fs_byte(name+name_cnt) != '\0')
	    name_cnt++;

	/* 如果名称长度超过了定义的最大长度，则返回NULL表示创建失败 */
    if(name_cnt>SEM_NAME_LEN)
	    return NULL;

	/* 
	 * 将信号量名称从用户态复制到内核态，存入kernelname数组中，
	 * 以便在内核中安全地使用该名称。
	 */
    for(i=0;i<name_cnt;i++)
	    kernelname[i]=get_fs_byte(name+i);

    /* 获取复制到内核态后的信号量名称的实际长度 */
    int name_len = strlen(kernelname);
    /* 临时变量，用于存储比较时信号量名称的长度 */
    int sem_name_len = 0;
    /* 指向信号量结构体的指针 */
    sem_t *p = NULL;  

    /* 判断信号量是否已经在信号量表中存在 */
    for(i=0;i<cnt;i++)
    {
        /* 首先比较信号量名称的长度 */
        sem_name_len = strlen(semtable[i].name);
        if(sem_name_len == name_len)
        {
            /* 如果长度相同，则进一步比较名称是否一致 */
            if( !strcmp(kernelname,semtable[i].name) )
            {
                /* 如果名称一致，则设置isExist标志并退出循环 */
                isExist = 1;
                break;
            }
        }
    }

    /* 如果信号量已经存在，则直接返回对应的信号量指针 */
    if(isExist == 1)
    {
        p = (sem_t*)(&semtable[i]);
        /* printk("find previous name!\n"); */
    }
    else   /* 如果信号量不存在，则进行创建 */
    {
        i=0;
        /* 将新的信号量名称复制到信号量表中 */
        for(i=0;i<name_len;i++)
        {
            semtable[cnt].name[i]=kernelname[i];
        }
        /* 设置新信号量的初始值 */
        semtable[cnt].value = value;
        /* 更新指向新创建信号量的指针 */
        p=(sem_t*)(&semtable[cnt]);
        /* printk("creat name!\n"); */
        /* 增加已创建信号量的计数 */
        cnt++;
     }
    /* 返回信号量指针 */
    return p;
}

/* 信号量P操作（等待操作） */
int sys_sem_wait(sem_t *sem)
{
    /* 关中断，阻止调度，确保临界区操作的原子性 */
    cli(); 
    /* 如果信号量值小于等于0，则当前进程进入睡眠，直到有资源可用 */
    while( sem->value <= 0 )        
        sleep_on(&(sem->queue));    
    /* 对信号量执行减1操作 */
    sem->value--;
    /* 开中断，允许其他进程运行 */
    sti();
    /* 成功完成操作 */
    return 0;
}

/* 信号量V操作（发布操作） */
int sys_sem_post(sem_t *sem)
{
    /* 关中断，阻止调度，确保临界区操作的原子性 */
    cli();
    /* 对信号量执行加1操作 */
    sem->value++;
    /* 如果信号量值小于等于1，则唤醒等待队列中的一个进程 */
    if( (sem->value) <= 1)
        wake_up(&(sem->queue));
    /* 开中断，允许其他进程运行 */
    sti();
    /* 成功完成操作 */
    return 0;
}

/* 删除信号量 */
int sys_sem_unlink(const char *name)
{
    /* 内核态使用的信号量名称缓冲区，长度应该足够大以容纳用户提供的名称 */
    char kernelname[100];   
    /* 标记信号量是否已经存在 */
    int isExist = 0;
    /* 循环计数器 */
    int i=0;
    /* 用户空间信号量名称的长度 */
    int name_cnt=0;

    /* 
     * 计算用户空间提供的信号量名称长度。
     * 使用get_fs_byte函数逐字节从用户空间复制到内核空间。
     */
    while( get_fs_byte(name+name_cnt) != '\0')
            name_cnt++;

    /* 如果名称长度超过了定义的最大长度，则返回NULL表示删除失败 */
    if(name_cnt>SEM_NAME_LEN)
            return NULL;

    /* 
     * 将信号量名称从用户态复制到内核态，存入kernelname数组中，
     * 以便在内核中安全地使用该名称。
     */
    for(i=0;i<name_cnt;i++)
            kernelname[i]=get_fs_byte(name+i);

    /* 获取复制到内核态后的信号量名称的实际长度 */
    int name_len = strlen(name);  /* 注意这里可能是原代码的一个bug，应该是strlen(kernelname) */
    /* 临时变量，用于存储比较时信号量名称的长度 */
    int sem_name_len =0;

    /* 判断是否存在，并定位赋值给cnt */
    for(i=0;i<cnt;i++)
    {
        /* 首先比较信号量名称的长度 */
        sem_name_len = strlen(semtable[i].name);
        if(sem_name_len == name_len)
        {
            /* 如果长度相同，则进一步比较名称是否一致 */
            if( !strcmp(kernelname,semtable[i].name))
            {
                /* 如果名称一致，则设置isExist标志并退出循环 */
                isExist = 1;
                break;
            }
        }
    }

    /* 如果信号量存在，则进行删除 */
    if(isExist == 1)
    {
        int tmp=0;
        /* 从定位到的cnt位置往后开始向前覆盖，实现删除 */
        for(tmp=i;tmp<=cnt;tmp++)  /* 注意这里的循环条件可能导致数组越界 */
        {
            semtable[tmp]=semtable[tmp+1];
        }
        /* 减少已创建信号量的计数 */
        cnt = cnt-1;  
        /* 成功删除信号量 */
        return 0;
    }
    else
        /* 如果信号量不存在，则返回错误码 */
        return -1;  
}