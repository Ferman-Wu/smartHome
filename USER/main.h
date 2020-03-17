#ifndef __MAIN_H
#define __MAIN_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "sdio_sdcard.h"
#include "ff.h"

#include "LD3320.h"
#include "LD3320Task.h"
#include "AppTask.h"
#include "ControlTask.h"
#include "StartTask.h"


/*======================外部接口======================*/
#define ON  0
#define OFF 1

#define TRUE  1
#define FALSE 0

/*======================内部函数======================*/
static void System_Init(void);
static void show_sdcard_info(void);
static void SD_Fatfs_Init(void);
#endif
