#include "main.h"
//任务句柄
TaskHandle_t StartTask_Handler;		//开始任务
TaskHandle_t LD3320Task_Handler;	//LD3320
TaskHandle_t AppTask_Handler;		//APP控制
TaskHandle_t ControlTask_Handler;	//实际操作任务
QueueHandle_t xHandleQueue = NULL;	//实际操作控制队列
//二值信号量句柄
SemaphoreHandle_t AppSemaphore;		//二值信号量句柄

void Start_Task(void)
{
	//创建开始任务
	xTaskCreate((TaskFunction_t)start_task,          //任务函数
				(const char *)"start_task",          //任务名称
				(uint16_t)START_STK_SIZE,            //任务堆栈大小
				(void *)NULL,                        //传递给任务函数的参数
				(UBaseType_t)START_TASK_PRIO,        //任务优先级
				(TaskHandle_t *)&StartTask_Handler); //任务句柄
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); //进入临界区
	
	//创建二值信号量
	AppSemaphore = xSemaphoreCreateBinary();
	
	/* 创建消息队列 */
	xHandleQueue = xQueueCreate(1,sizeof(u8));
	
	if(xHandleQueue == NULL)     /* 如果消息队列创建成功 */
	{
		printf("Queue create fail!");
		while(1);
	}
	printf("Queue create success!\r\n");
	
	//创建LD3320_Task任务
    xTaskCreate((TaskFunction_t)LD3320_Task,
                (const char *)"LD3320_Task",
                (uint16_t)LD3320_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LD3320_TASK_PRIO,
                (TaskHandle_t *)&LD3320Task_Handler);
	//创建App控制任务
    xTaskCreate((TaskFunction_t)App_Task,
                (const char *)"App_Task",
                (uint16_t)APP_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)APP_TASK_PRIO,
                (TaskHandle_t *)&AppTask_Handler);
	//创建实际控制任务
    xTaskCreate((TaskFunction_t)Control_Task,
                (const char *)"Control_Task",
                (uint16_t)CONTROL_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)CONTROL_TASK_PRIO,
                (TaskHandle_t *)&ControlTask_Handler);
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}
