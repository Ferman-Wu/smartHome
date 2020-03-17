#include "main.h"

#define APP_TASK_INIT_TIME 300
#define TASK_DELAY_MS 200
#define TICK_WAIT     50
//用户修改内容 - 打印信息
static const u8 ASR_LEN = 7;    //指令数目，printfCmd数组和MP3Name数组一维数值-1
const char* printfCmd[] = {
	"No command\r\n",
	"system name\r\n",
	"Running water light success\r\n",
	"Light flashing success\r\n",
	"button success\r\n",
	"all off success\r\n",
	"state success\r\n",
	"hello success\r\n"
};

const char* MP3Name[] = {
	"tingBuQing.mp3",
	"zhuRenWoZai.mp3",
	"liuShuiDeng.mp3",
	"ShanShuo.mp3",
	"",
	"",
	"",
	""
};

static u8 preResVal = 0;

void Control_Task(void *pvParameters)
{
	u8 resVal;
	
	portBASE_TYPE xStatus;

	printf("Control task start\r\n");
	
	while(1)
	{
		//等待消息 等待TICK_WAIT个节拍(ms)
		xStatus = xQueueReceive( xHandleQueue, &resVal, (TickType_t)TICK_WAIT ); 
		
		if(xStatus==pdPASS)                                                      //如果成功接收大到消息
		{
			//打印消息内容
			if(resVal != 0)
			{
				//ASR语音控制最多50条，51开始为APP特有测试指令
				if(resVal <= ASR_LEN && resVal <= 50 )
				{
					printf(printfCmd[resVal]);
					LD_Play_MP3(MP3Name[resVal], 1);
				}
				
				preResVal = resVal;
			}
			Board_text(resVal);
		}
		else                                                                     //如果没有接收到消息
		{
			if(preResVal != 0)
			{
				Board_text(preResVal);
				vTaskDelay(10);
			}
		}
	}	
	
}

u8 get_ASR_Len(void)	//获取指令数量
{
	return ASR_LEN;
}

static void Board_text(u8 Code_Val)
{
	static u8 preCodeVal = 0;
	switch(Code_Val)  //对结果执行相关操作
	{
		case CODE_NAME:
			break;
		case CODE_LSD:  //命令“流水灯”
			Glide_LED();
			break;
		case CODE_SS:	  //命令“闪烁”
			Flicker_LED();
			break;
		case CODE_AJCF:	//命令“按键触发”
			Key_LED();
			break;
		case CODE_QM:		//命令“全灭”
			Off_LED();
			break;
		case CODE_JT:		//命令“状态”
			Jt_LED();
			break;
		case CODE_NH:		//命令“你好”
			Flicker_LED();
			break;
		case LED1ON: 
			if(LED1 != ON)
			{
				LED1=ON;
			}
			break;
		case LED1OFF:
			if(LED1 != OFF)
			{
				LED1=OFF;
			}
			break;
		case LED0ON:
			if(LED0 != ON)
			{
				LED0=ON;
			}
			break;
		case LED0OFF:
			if(LED0 != OFF)
			{
				LED0=OFF;
			}
			break;
		
		default:	
			LD_Play_MP3(MP3Name[0], 1);
			preResVal = preCodeVal;	//回退上一个正常状态
			return;
	}	
	preCodeVal = Code_Val;	//保存上一个正常状态
}

static void Glide_LED(void)
{
	LED0=ON;			
	LED1=OFF;			
	delay_ms(500);
	LED0=OFF;			
	LED1=ON;			
	delay_ms(500);
}

static void Flicker_LED(void)
{
	LED0=ON;			
	LED1=ON;			
	delay_ms(500);
	LED0=OFF;			
	LED1=OFF;			
	delay_ms(1500);
}

static void Key_LED(void)
{
	LED0=ON;		
	delay_ms(500);
	LED0=OFF;			
	delay_ms(500);
}

static void Off_LED(void)
{
	LED1=OFF;
	LED0=OFF;
}

static void Jt_LED(void)
{
	LED1=ON;		
	delay_ms(500);
	LED1=OFF;		
	delay_ms(500);
}
