#ifndef __STORAGETASK_H
#define __STORAGETASK_H

#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"

#include "fatfs.h" 
#include "stdio.h"
#include "string.h"
#include "rtc.h"
#include "adcTask.h"
#include "displayTask.h"

#define STORAGE_CMD_QUEUE_LEN   5
#define STORAGE_DATA_QUEUE_LEN   8

// 存储命令
typedef enum {
    CMD_IDLE = 0,   // 无命令
    CMD_START,      // 开始存储
    CMD_STOP        // 停止存储
} StorageCmd_t;

extern QueueHandle_t g_StorageQueue;
extern QueueHandle_t g_StorageDataQueue;


void Storage_Init(void);
void Storage_Task(void *argument);

#endif
