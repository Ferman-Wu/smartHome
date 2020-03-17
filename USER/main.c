#include "main.h"

/** 
* @project  智能家居语音识别系统（下位机）
* @brief    基于STM32F407ZG，语音识别模块为LD3320
* @details  This is the detail description. 
* @author   漫枫
* @par 		qq_19257541(CSDN ID)
* @par 		page:https://me.csdn.net/qq_19257541
* @date     2020.3.9
* @version  V1.4 
*/ 
FATFS Fat_SD;//挂载SD

int main(void)
{ 
	System_Init();			//系统初始化
	printf("\r\nSystem start...\r\n");
	LD_Play_MP3("SystemStart.mp3", 1);
	Start_Task();			//开始任务						
	vTaskStartScheduler();  //开启任务调度
	
	while (1);
}

//系统初始化
static void System_Init(void){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);	//设置系统中断优先级分组4
	delay_init(168);							   	//初始化延时函数
	uart_init(115200);     							//初始化串口
	LED_Init();		        						//初始化LED端口	       
	LD3320_init();									//LD3320执行函数
	delay_ms(100);
	
	printf("\r\n---\r\n");
	SD_Fatfs_Init();
}

static void SD_Fatfs_Init(void)
{
	FIL fil;
	char line[100];
	while(SD_Init())
	{
		//检测不到SD卡
		printf("SD Card Error!\r\n");
		delay_ms(500);					
		printf("Please Check! \r\n");
		delay_ms(500);
		LED0=!LED0;//DS0闪烁
	}
	
	//检测SD卡成功 
	printf("SD Card OK\r\n");
	
  	f_mount(&Fat_SD,"0:",1); 					//挂载SD卡
	f_open(&fil, "first.txt", FA_READ);
	while (f_gets(line, sizeof line, &fil)) {
        printf(line);
    }
	f_close(&fil);
}
