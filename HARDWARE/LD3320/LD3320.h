#ifndef __LD3320_H
#define __LD3320_H
#include "sys.h"

///以下三个状态定义用来记录程序是在运行ASR识别还是在运行MP3播放
#define LD_MODE_IDLE		0x00
#define LD_MODE_ASR_RUN		0x08
#define LD_MODE_MP3		 	0x40

///以下五个状态定义用来记录程序是在运行ASR识别过程中的哪个状态
#define LD_ASR_NONE					0x00	//表示没有在作ASR识别
#define LD_ASR_RUNING				0x01	//表示LD3320正在作ASR识别中
#define LD_ASR_FOUNDOK			0x10	//表示一次识别流程结束后，有一个识别结果
#define LD_ASR_FOUNDZERO 		0x11	//表示一次识别流程结束后，没有识别结果
#define LD_ASR_ERROR	 			0x31	//表示一次识别流程中LD3320芯片内部出现不正确的状态
#define LD_MP3_PLAY				0xA1

#define CLK_IN   					22/* 用户需要根据时钟修改这个值 */
#define LD_PLL_11					(u8)((CLK_IN/2.0)-1)
#define LD_PLL_MP3_19			0x0f
#define LD_PLL_MP3_1B			0x18
#define LD_PLL_MP3_1D   	(u8)(((90.0*((LD_PLL_11)+1))/(CLK_IN))-1)

#define LD_PLL_ASR_19 		(u8)(CLK_IN*32.0/(LD_PLL_11+1) - 0.51)
#define LD_PLL_ASR_1B 		0x48
#define LD_PLL_ASR_1D 		0x1f

//麦克风音量
#define MIC_VOL 0x4C

//MP3播放相关
#define        RESUM_OF_MUSIC               0x01
#define        CAUSE_MP3_SONG_END           0x20

#define        MASK_INT_SYNC				0x10
#define        MASK_INT_FIFO				0x04
#define    	   MASK_AFIFO_INT				0x01
#define        MASK_FIFO_STATUS_AFULL		0x08

#define LD3320_CS   PAout(4)
#define LD3320_SCK  PAout(5)
#define LD3320_MISO PAout(6)
#define LD3320_MOSI PAout(7)
#define LD3320_IRQ  PBout(12)
#define LD3320_WR 	PBout(13)
#define LD3320_RST  PBout(14)
/*======================外部接口======================*/
void LD3320_init(void);
//延时函数
void LD3320_delay(unsigned long uldata);
//重置函数
void LD3320_reset(void);

/*======================内部函数======================*/
//相关初始化
static void LD3320_GPIO_Cfg(void);	
static void LD3320_EXTI_Cfg(void);
static void LD3320_SPI_Cfg(void);



#endif


