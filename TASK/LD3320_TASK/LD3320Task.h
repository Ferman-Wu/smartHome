#ifndef LD3320_TASK_H
#define LD3320_TASK_H
//识别码（客户修改处）不能为0
#define CODE_NAME	1	//流水灯
#define CODE_LSD	2	//流水灯
#define CODE_SS	  	3	//闪烁
#define CODE_AJCF	4	//按键触发
#define CODE_QM	  	5	//全灭
#define CODE_JT		6  	//状态
#define CODE_NH		7  	//你好


/*======================外部接口======================*/
//任务函数
void LD3320_Task(void *pvParameters);
//MP3播放
void LD_Play_MP3(const char *path,const u8 Volume);

/*======================内部函数======================*/
//用户修改函数
void LD3320_Running(void);
u8 LD_AsrAddFixed(void);

//中断处理函数
void ProcessInt(void);

//中间层
u8 RunASR(void);
void LD_AsrStart(void);
u8 LD_Check_ASRBusyFlag_b2(void);

//寄存器操作
u8 spi_send_byte(u8 byte);
void LD_WriteReg(u8 add,u8 data);
u8 LD_ReadReg(u8 reg_add);
u8 LD_GetResult(void);
u8 LD_AsrRun(void);
void LD_Init_Common(void);
void LD_Init_ASR(void);

//MP3操作
void LD_AdjustMIX2SPVolume(u8 val);
void LD_Init_MP3(void);
void LD_Play(void);
void Fill_the_Fifo(void);
void MP3_END_Reset(void);
void LD_ReloadMp3Data(void);
#endif
