#include "main.h"
#include "string.h"
#define LD3320_TASK_INIT_TIME 200
#define TASK_DELAY_MS 100
/************************************************************************************
//	nAsrStatus 用来在main主程序中表示程序运行的状态，不是LD3320芯片内部的状态寄存器
//	LD_ASR_NONE:			表示没有在作ASR识别
//	LD_ASR_RUNING：		表示LD3320正在作ASR识别中
//	LD_ASR_FOUNDOK:		表示一次识别流程结束后，有一个识别结果
//	LD_ASR_FOUNDZERO:	表示一次识别流程结束后，没有识别结果
//	LD_ASR_ERROR:			表示一次识别流程中LD3320芯片内部出现不正确的状态
*********************************************************************************/
static u8 nAsrStatus = 0;
static u8 nLD_Mode = LD_MODE_IDLE;//用来记录当前是在进行ASR识别还是在播放MP3
static u8 ucRegVal;
static u8 nAsrRes;
FIL fsrc;//文件变量
DWORD nMp3Size=0;;//MP3的大小
DWORD nMp3Pos; //MP3播放的长度
u8 bMp3Play=0;//是否进行着MP3播放
u8 ucSPVol=15; // MAX=15 MIN=0		//	Speaker喇叭输出的音量

void LD3320_Task(void *pvParameters)
{
	//获取创建任务时传入的参数	
	vTaskDelay(LD3320_TASK_INIT_TIME);
	printf("LD3320 task start\r\n");
	nAsrRes=0;
	while(1)
	{
		LD3320_Running();
		//系统延时
		vTaskDelay(TASK_DELAY_MS);
	}
	
}

//用户修改
void LD3320_Running(void)
{
	static u8 isAwake = 0;
	portBASE_TYPE xStatus;
	switch(nAsrStatus)
	{
		case LD_ASR_RUNING:
		case LD_ASR_ERROR:
		case LD_MP3_PLAY:
				 break;
		case LD_ASR_NONE:
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)//启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算
				{		
					nAsrStatus = LD_ASR_ERROR;   //ASR流程错误
					printf("ASR error\r\n");
				}
				else
				{
					//printf("ASR success\r\n");
				}
				break;
		case LD_ASR_FOUNDOK:
				nAsrRes = LD_GetResult( );//一次ASR识别流程结束，去取ASR识别结果										 
				printf("id code:%d, ", nAsrRes);
				if(nAsrRes == CODE_NAME)
				{
					printf("hear my name\r\n");
					LD_Play_MP3("zhuRenWoZai.mp3", 1);
					isAwake = 1;
				}
				else
				{
					if(!isAwake)	//没有唤醒
					{
						printf("I'm sleep\r\n");
						nAsrStatus = LD_ASR_NONE;
						return;
					}
					if(nAsrRes <= get_ASR_Len())	//判断是否在指令范围内，若
					{
						printf("I'm reset\r\n");
						isAwake = 0;	//重置状态
					}
					//堵塞任务等待消息队列的消息
					xStatus = xQueueOverwrite(xHandleQueue, &nAsrRes);
					
					//如果发送消息失败
					if( xStatus != pdPASS )                                        
					{
					  printf("Data can't to send to the queue.\r\n");
					}		
				}
				
				nAsrStatus = LD_MP3_PLAY;			
				break;
		case LD_ASR_FOUNDZERO:
		default:
			nAsrStatus = LD_ASR_NONE;
			break;
		}//switch
	
		
	
}

//中断处理函数
void ProcessInt(void)
{
	u8 nAsrResCount=0;
	u8 ucHighInt;
	u8 ucLowInt;
	ucRegVal = LD_ReadReg(0x2B);

	if(nLD_Mode == LD_MODE_ASR_RUN&&(!bMp3Play))
	{
		// 语音识别产生的中断
		//（有声音输入，不论识别成功或失败都有中断）
		LD_WriteReg(0x29,0) ;
		LD_WriteReg(0x02,0) ;

		if((ucRegVal & 0x10) && LD_ReadReg(0xb2)==0x21 && LD_ReadReg(0xbf)==0x35)		
		{	 
				nAsrResCount = LD_ReadReg(0xba);

				if(nAsrResCount>0 && nAsrResCount<=4) 
				{
					nAsrStatus=LD_ASR_FOUNDOK; 				
				}
				else
				{
					nAsrStatus=LD_ASR_FOUNDZERO;
				}	
		}
		else
		{
			nAsrStatus=LD_ASR_FOUNDZERO;//执行没有识别
		}

		LD_WriteReg(0x2b,0);
		LD_WriteReg(0x1C,0);//写0:ADC不可用
		return;
	}
	
	//MP3播放模式
	ucHighInt=LD_ReadReg(0x29);
	ucLowInt=LD_ReadReg(0x02);
	LD_WriteReg(0x29,0) ;		//中断允许 FIFO 中断允许 0表示不允许 
	LD_WriteReg(0x02,0) ;		// FIFO中断允许	 FIFO_DATA FIFO_EXT中断  不允许
	//声音播放完毕
	if(LD_ReadReg(0xBA)&CAUSE_MP3_SONG_END)
	{
		LD_WriteReg(0x2B, 0); 
		LD_WriteReg(0xBA,0x00);
		LD_WriteReg(0xBC,0x00);
		LD_WriteReg(0x08,0x01); 
		LD_WriteReg(0x08,0x00); 
		LD_WriteReg(0x33,0x00); 
		bMp3Play=0;//标志播放结束
		MP3_END_Reset();
		return ;
	}
	//声音数据已发送完毕。
	if(nMp3Pos>=nMp3Size)
	{ 
		LD_WriteReg(0xBC, 0x01); 
		LD_WriteReg(0x29, 0x10);
		bMp3Play=0;//标志播放结束
		MP3_END_Reset();
		return; 
	}
	//声音数据暂时将要用完，需要放入新的数据。	
	LD_ReloadMp3Data();		//播放MP3
	LD_WriteReg(0x29,ucHighInt);
	LD_WriteReg(0x02,ucLowInt) ; 			
	nLD_Mode = LD_MODE_ASR_RUN;//回到语音识别状态
	nAsrStatus = LD_ASR_NONE;
}

//添加关键词
u8 LD_AsrAddFixed(void)
{
	u8 k, flag;
	u8 nAsrAddLength;
	#define DATE_A 7    //数组二维数值
	#define DATE_B 20	//数组一维数值
	//添加关键词，用户修改
	u8  sRecog[DATE_A][DATE_B] = {
		    "xiao peng you",\
			"liu shui deng",\
			"shan shuo",\
			"an jian chu fa",\
			"quan mie",\
			"zhuang tai",\
			"ni hao"\
	};	
	u8  pCode[DATE_A] = {
		CODE_NAME,  \
		CODE_LSD,	\
		CODE_SS,	\
		CODE_AJCF,  \
		CODE_QM,	\
		CODE_JT	,	\
		CODE_NH\
	};	//添加识别码，用户修改
	flag = 1;
	for (k=0; k<DATE_A; k++)
	{			
		if(LD_Check_ASRBusyFlag_b2() == 0)
		{
			flag = 0;
			printf("LD_Check_ASRBusyFlag_b2 is error\r\n");
			break;
		}

		LD_WriteReg(0xc1, pCode[k] );
		LD_WriteReg(0xc3, 0);
		LD_WriteReg(0x08, 0x04);
		LD3320_delay(1);
		LD_WriteReg(0x08, 0x00);
		LD3320_delay(1);

		for (nAsrAddLength=0; nAsrAddLength<DATE_B; nAsrAddLength++)
		{
			if (sRecog[k][nAsrAddLength] == 0)
				break;
			LD_WriteReg(0x5, sRecog[k][nAsrAddLength]);
		}
		LD_WriteReg(0xb9, nAsrAddLength);
		LD_WriteReg(0xb2, 0xff);
		LD_WriteReg(0x37, 0x04);
	}	 
	return flag;
}

//用户修改 end

//中间层
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line12)!= RESET ) 
	{
		printf("enter IRQ 12\r\n");
		ProcessInt(); 
 		printf("out IRQ 12\r\n");	
		EXTI_ClearFlag(EXTI_Line12);
		EXTI_ClearITPendingBit(EXTI_Line12);//清除LINE上的中断标志位  
	} 
}

u8 RunASR(void)
{
	u8 i=0;
	u8 asrflag=0;
	for (i=0; i<5; i++)		//防止由于硬件原因导致LD3320芯片工作不正常，所以一共尝试5次启动ASR识别流程
	{
		LD_AsrStart();			//初始化ASR
		LD3320_delay(100);
		if (LD_AsrAddFixed()==0)	//添加关键词语到LD3320芯片中
		{
			printf("LD_AsrAddFixed is error\r\n");
			LD3320_reset();				//LD3320芯片内部出现不正常，立即重启LD3320芯片
			LD3320_delay(50);	//并从初始化开始重新ASR识别流程
			continue;
		}
		LD3320_delay(10);
		if (LD_AsrRun() == 0)
		{
			printf("LD_AsrRun is error\r\n");
			LD3320_reset();			 //LD3320芯片内部出现不正常，立即重启LD3320芯片
			LD3320_delay(50);//并从初始化开始重新ASR识别流程
			continue;
		}
		asrflag=1;
		break;						//ASR流程启动成功，退出当前for循环。开始等待LD3320送出的中断信号
	}	
	return asrflag;
}


void LD_AsrStart(void)
{
	LD_Init_ASR();
}

u8 LD_Check_ASRBusyFlag_b2(void)
{
	u8 j;
	u8 flag = 0;
	for (j=0; j<10; j++)
	{
		if (LD_ReadReg(0xb2) == 0x21)
		{
			flag = 1;
			break;
		}
//		printf("reg(0xb2) data is %x\r\n",LD_ReadReg(0xb2));
		LD3320_delay(10);		
	}
	return flag;
}
//中间层 end

//寄存器操作
u8 spi_send_byte(u8 byte)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1,byte);
	while (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}

void LD_WriteReg(u8 add,u8 data)
{
	LD3320_CS=0;
	__nop();
	__nop();
	__nop();
	LD3320_WR=0;
	spi_send_byte(0x04);
	spi_send_byte(add);
	spi_send_byte(data);
	LD3320_CS=1;
}

u8 LD_ReadReg(u8 reg_add)
{
	u8 i;
	LD3320_CS=0;
	__nop();
	__nop();
	__nop();
	LD3320_WR=0;
	spi_send_byte(0x05);
	spi_send_byte(reg_add);
	i=spi_send_byte(0x00);
	LD3320_CS=1;
	return(i);
}

u8 LD_GetResult(void)
{
	return LD_ReadReg(0xc5);
}

u8 LD_AsrRun(void)
{
	LD_WriteReg(0x35, MIC_VOL);//设置MIC音量
	LD_WriteReg(0x1C, 0x09);   //ADC开关写入保留字
	LD_WriteReg(0xBD, 0x20);   //初始化控制寄存器写入保留字
	LD_WriteReg(0x08, 0x01);   //清除FIFO_DATA
	LD3320_delay( 5 );
	LD_WriteReg(0x08, 0x00);   //清除之后要再写一次0x00
	LD3320_delay( 5);

	if(LD_Check_ASRBusyFlag_b2() == 0)  //DSP忙碌
	{
		return 0;  //返回失败状态
	}

	LD_WriteReg(0xB2, 0xff);	//DSP忙碌状态寄存器初始化
	LD_WriteReg(0x37, 0x06);  //通知DSP开始语音识别
	LD3320_delay(5);
	LD_WriteReg(0x1C, 0x0b);  //ADC开关,MIC输入可用
	LD_WriteReg(0x29, 0x10);  //同步中断允许
	LD_WriteReg(0xBD, 0x00);  //启动为ASR模块
	return 1;
}



void LD_Init_Common(void)
{
	LD_ReadReg(0x06);  
	LD_WriteReg(0x17, 0x35); //对LD3320进行软复位
	LD3320_delay(5);
	LD_ReadReg(0x06);  

	LD_WriteReg(0x89, 0x03);  //模拟电路控制初始化
	LD3320_delay(5);
	LD_WriteReg(0xCF, 0x43);  //内部省电模式初始化
	LD3320_delay(5);
	LD_WriteReg(0xCB, 0x02);
	
	/*PLL setting*/
	LD_WriteReg(0x11, LD_PLL_11); 	
	if (nLD_Mode == LD_MODE_MP3)
	{
		LD_WriteReg(0x1E, 0x00); 
		//!!注意,下面三个寄存器,会随晶振频率变化而设置不同
		//!!注意,请根据使用的晶振频率修改参考例程中的 CLK_IN
		LD_WriteReg(0x19, LD_PLL_MP3_19);   
		LD_WriteReg(0x1B, LD_PLL_MP3_1B);   
		LD_WriteReg(0x1D, LD_PLL_MP3_1D);
	}
	else
	{
		LD_WriteReg(0x1E,0x00);
		//!!注意,下面三个寄存器,会随晶振频率变化而设置不同
		//!!注意,请根据使用的晶振频率修改参考例程中的 CLK_IN
		LD_WriteReg(0x19, LD_PLL_ASR_19); 
		LD_WriteReg(0x1B, LD_PLL_ASR_1B);		
	  LD_WriteReg(0x1D, LD_PLL_ASR_1D);
	}
	LD3320_delay(5);
	
	LD_WriteReg(0xCD, 0x04); //初始化时允许DSP休眠
	LD_WriteReg(0x17, 0x4c); //使DSP休眠
	LD3320_delay(1);
	LD_WriteReg(0xB9, 0x00); //添加识别字初始化,每添加一条语句后要设定一次
	LD_WriteReg(0xCF, 0x4F); //MP3初始化和ASR初始化
	LD_WriteReg(0x6F, 0xFF); //对芯片进行初始化
}

void LD_Init_ASR(void)
{
	nLD_Mode=LD_MODE_ASR_RUN;  //默认ASR识别运行
	LD_Init_Common();

	LD_WriteReg(0xBD, 0x00);
	LD_WriteReg(0x17, 0x48);	
	LD3320_delay(5);
	LD_WriteReg(0x3C, 0x80);    
	LD_WriteReg(0x3E, 0x07);
	LD_WriteReg(0x38, 0xff);    
	LD_WriteReg(0x3A, 0x07);
	LD_WriteReg(0x40, 0);          
	LD_WriteReg(0x42, 8);
	LD_WriteReg(0x44, 0);    
	LD_WriteReg(0x46, 8); 
	LD3320_delay(1);
}
//寄存器操作 end

//MP3播放
void LD_Play_MP3(const char *path,const u8 Volume)
{
	//判断path是否为空
	if(path == NULL || strlen(path) == 0)
	{
		nAsrStatus = LD_ASR_NONE;
		printf("play path is null \r\n");
		return;
	}
	//兼容APP指令控制，关闭ASR识别
	nAsrStatus = LD_MP3_PLAY;
	LD_WriteReg(0x2b,0);
	LD_WriteReg(0x1C,0);//写0:ADC不可用
	
	bMp3Play = 1;
	while(!(f_open(&fsrc, path,FA_READ) == FR_OK));//打开文件
	nMp3Size = fsrc.fsize;
	LD_Init_MP3();									//将LD3320初始化为播放MP3模式
	LD_AdjustMIX2SPVolume(Volume);	//设置耳机音量  
	LD_Play();											//开始播放
}

/*
Name : LD_AdjustMIX2SPVolume
Function : 调节喇叭音量
						val :  2~5位有效
			7 6 5 4 3 2 1 0
			x x 0 0 0 0 x x  	音量最大
			x x 1 1 1 1 x x		音量最小
*/
void LD_AdjustMIX2SPVolume(u8 val) 
{
	ucSPVol = val; //记录当前的喇叭音量
	val = ((15-val)&0x0f) << 2; 
	LD_WriteReg(0x8E, val | 0xc3);//调节音量
	LD_WriteReg(0x87, 0x78);//使调节的音量有效
}

void LD_Init_MP3(void)
{
	nLD_Mode=LD_MODE_MP3;	//当前进行MP3模式
	LD_Init_Common();//通用初始化
	
	LD_WriteReg(0xBD,0x02);//启动MP3模块
	LD_WriteReg(0x17,0x48);//激活DSP
	delay_ms(10);//延时1ms
	LD_WriteReg(0x85,0x52);//初始化
	LD_WriteReg(0x8F,0x00);//LineOut选择
	LD_WriteReg(0x81,0x00);//耳机左声音调节为最大音
	LD_WriteReg(0x83,0x00);//耳机右声音调节为最大音
	LD_WriteReg(0x8E,0xff);//喇叭输出音量最小
	LD_WriteReg(0x8D,0xff);//内部增益
	delay_ms(1);//延时1ms
	LD_WriteReg(0x87,0xff);//模拟电路控制初始化
	LD_WriteReg(0x89,0xff);//模拟电路控制
	delay_ms(1);//延时1ms
	LD_WriteReg(0x22,0x00);//(LowerBoundary L)
	LD_WriteReg(0x23,0x00);//(LowerBoundary H)
	LD_WriteReg(0x20,0xef);//(UpperBoundary L)
	LD_WriteReg(0x21,0x07);//(UpperBoundary H)
	LD_WriteReg(0x24,0x77);//(MCU water mark L)
	LD_WriteReg(0x25,0x03);//(MCU water mark H)
	LD_WriteReg(0x26,0xbb);//(DSP water mark L)
	LD_WriteReg(0x27,0x01);//(DSP water mark H)
}

void LD_Play(void)
{
	
	nMp3Pos=0; //播放长度1
	//如果超出了总的MP3数据长度
	if(nMp3Pos>=nMp3Size)
		return;

	Fill_the_Fifo();//可产生中断 然后调用 Reload进行播放
	LD_WriteReg(0xBA, 0x00);//中断辅助信息 设置为00
	LD_WriteReg(0x17, 0x48);//激活DSP
	LD_WriteReg(0x33, 0x01);//开始播放时写入01H
	LD_WriteReg(0x29, 0x04);//允许同步中断
	LD_WriteReg(0x02, 0x01);//允许FIFO_DATA中断
	LD_WriteReg(0x85, 0x5A);//MP3播放改变内部增益
	//EX0=1;开外部中断0
	EXTI_ClearITPendingBit(EXTI_Line12);
}

void Fill_the_Fifo(void)
{
  u8 ucStatus;
	int i = 0;
	ucStatus = LD_ReadReg(0x06);
	//fifo是否满了
	while ( !(ucStatus&MASK_FIFO_STATUS_AFULL))
	{
		LD_WriteReg(0x01,0xff);
		i++;
		ucStatus = LD_ReadReg(0x06);
	}
}

void LD_ReloadMp3Data(void)
{
	u8 ucStatus;
	u8 val;
	u32 i;

	UINT br;

	//Read Command
	ucStatus=LD_ReadReg(0x06);
	//播放 FIFO_Status_Allfull 将会引发中断 再次调用这个函数 来循环读取 完整的播放
	while(!(ucStatus&MASK_FIFO_STATUS_AFULL)&&nMp3Pos<nMp3Size)
	{
		//从SD卡读取MP3数据
		f_read(&fsrc,&val,1,&br);
		nMp3Pos++;
		LD_WriteReg(0x01,val);//送入数据
		ucStatus=LD_ReadReg(0x06);
	}
	//播放退出 在此需要判断是否播放完毕 防止是数据口满了的退出的情况
	if(nMp3Pos>=nMp3Size)
	{
		LD_WriteReg(0xBC, 0x01);
		LD_WriteReg(0x29, 0x10);
		//等待芯片播放 如果时间超过则强制关闭等待MP3播放完毕
		i=600000;
		while(!(LD_ReadReg(0xBA)&CAUSE_MP3_SONG_END)&&i>0)
		{
			i--;
			//printf("Play busy\r\n");
		}
		bMp3Play=0;					// 声音数据全部播放完后，修改bMp3Play的变量
		printf("\r\nPlay Over\r\n");//播放完毕
		LD_WriteReg(0x2B,  0);
		LD_WriteReg(0xBA, 0);	
		LD_WriteReg(0xBC,0x0);	

		LD_WriteReg(0x08,1);

		LD_WriteReg(0x08,0);
		LD_WriteReg(0x33, 0);
		MP3_END_Reset();
	}
}

void MP3_END_Reset(void)
{
	f_close(&fsrc);//关闭文件
	nLD_Mode = LD_MODE_ASR_RUN;
	nAsrStatus = LD_ASR_NONE;
}





