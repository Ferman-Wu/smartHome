#ifndef __STARTTASK_H
#define __STARTTASK_H

void Start_Task(void);

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define LD3320_TASK_PRIO	3
//任务堆栈大小	
#define LD3320_STK_SIZE 	512  


//任务优先级
#define APP_TASK_PRIO		4
//任务堆栈大小	
#define APP_STK_SIZE 		128


//任务优先级
#define CONTROL_TASK_PRIO		8
//任务堆栈大小	
#define CONTROL_STK_SIZE 		512

extern QueueHandle_t xHandleQueue;
extern SemaphoreHandle_t AppSemaphore;


#endif
