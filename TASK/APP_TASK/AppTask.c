#include "main.h"
#include "string.h"
#include "malloc.h"
#define APP_TASK_INIT_TIME 300
#define TASK_DELAY_MS 200
void AppCommandRunning(void);
void App_Task(void *pvParameters)
{
	BaseType_t err=pdFALSE;
	
	vTaskDelay(APP_TASK_INIT_TIME);
	printf("App task start\r\n");
	
	while(1)
	{
		if(AppSemaphore!=NULL)
		{
			err=xSemaphoreTake(AppSemaphore,portMAX_DELAY);	//获取信号量
			if(err==pdTRUE)										//获取信号量成功
			{
				//AppCotrolRunning();	
				AppCommandRunning();
			}
		}
		else if(err==pdFALSE)
		{
			vTaskDelay(10);      //延时10ms，也就是10个时钟节拍	
		}
	}
	
}

void AppCommandRunning(void)
{
	u8 len=0;
	u16 i;
	u8 cmd;
	portBASE_TYPE xStatus;
	
	len=USART_RX_STA&0x3fff;				//得到此次接收到的数据长度
	for(i = 0; i < len; i++)
	{
		if(USART_RX_BUF[i] == 0xA1)
		{
			cmd = USART_RX_BUF[i+1];
			
			//堵塞任务等待消息队列的消息
			xStatus = xQueueOverwrite(xHandleQueue, &cmd);
			
			//如果发送消息失败
			if( xStatus != pdPASS )                                        
			{
			  printf("Data can't to send to the queue.\r\n");
			}
			break;
		}
		
	}
	USART_RX_STA=0;
}

static void AppCotrolRunning(void)
{
	u8 len=0;
	u8 CommandValue=COMMANDERR;
	u8 *CommandStr;
	portBASE_TYPE xStatus;
	
	len=USART_RX_STA&0x3fff;				//得到此次接收到的数据长度
	CommandStr=mymalloc(SRAMIN,len+1);		//申请内存
	sprintf((char*)CommandStr,"%s",USART_RX_BUF);
	CommandStr[len]='\0';					//加上字符串结尾符号
	LowerToCap(CommandStr,len);				//将字符串转换为大写		
	CommandValue=CommandProcess(CommandStr);//命令解析
	if(CommandValue!=COMMANDERR)
	{
		if(CommandValue > 50)
		{
			printf("Command:%s\r\n",CommandStr);
		}
		//堵塞任务等待消息队列的消息
		xStatus = xQueueOverwrite(xHandleQueue, &CommandValue);
		
		//如果发送消息失败
		if( xStatus != pdPASS )                                        
		{
		  printf("Data can't to send to the queue.\r\n");
		}
	}
	else
	{
		printf("Error command!!\r\n");
	}
	USART_RX_STA=0;
	memset(USART_RX_BUF,0,USART_REC_LEN);	//串口接收缓冲区清零
	myfree(SRAMIN,CommandStr);				//释放内存
	
}

//命令处理函数，将字符串命令转换成命令值
//str：命令
//返回值: 0XFF，命令错误；其他值，命令值
static u8 CommandProcess(u8 *str)
{
	u8 CommandValue=COMMANDERR;
	if(strcmp((char*)str,"LED1ON")==0) CommandValue=LED1ON;
	else if(strcmp((char*)str,"LED1OFF")==0) CommandValue=LED1OFF;
	else if(strcmp((char*)str,"LED0ON")==0) CommandValue=LED0ON;
	else if(strcmp((char*)str,"LED0OFF")==0) CommandValue=LED0OFF;
	return CommandValue;
}

//将字符串中的小写字母转换为大写
//str:要转换的字符串
//len：字符串长度
static void LowerToCap(u8 *str,u8 len)
{
	u8 i;
	for(i=0;i<len;i++)
	{
		if((96<str[i])&&(str[i]<123))	//小写字母
		str[i]=str[i]-32;				//转换为大写
	}
}




