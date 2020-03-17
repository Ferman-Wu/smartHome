#ifndef __APP_TASK_H
//用于命令解析用的命令值，用于测试
#define LED1ON	51
#define LED1OFF	52
#define LED0ON	53
#define LED0OFF	54
#define COMMANDERR	0XFF
/*======================外部接口======================*/
void App_Task(void *pvParameters);

/*======================内部函数======================*/
static void AppCotrolRunning(void);
static u8 CommandProcess(u8 *str);
static void LowerToCap(u8 *str,u8 len);
#endif
