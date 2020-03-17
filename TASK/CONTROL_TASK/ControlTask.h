#ifndef __CONTROL_TASK_H
#define __CONTROL_TASK_H
/*======================外部接口======================*/
void Control_Task(void *pvParameters);
u8 get_ASR_Len(void);

/*======================内部函数======================*/
static void Board_text(u8 Code_Val);
//语音执行操作函数
static void Glide_LED(void);
static void Flicker_LED(void);
static void Key_LED(void);
static void Off_LED(void);
static void Jt_LED(void);
#endif
