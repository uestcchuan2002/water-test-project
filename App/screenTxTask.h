#ifndef __SCREENTXTASK_H
#define __SCREENTXTASK_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

#define SCREEN_TX_MAX_LEN      128
#define SCREEN_TX_QUEUE_LEN    16

typedef struct
{
    uint8_t data[SCREEN_TX_MAX_LEN];
    uint16_t len;
} ScreenTxMsg_t;

extern QueueHandle_t screenTxQueue;
extern SemaphoreHandle_t screenTxDoneSem;

void ScreenTx_Init(void);
void ScreenTx_Task(void *argument);
void Screen_SendCmd(const char *cmd);

#endif
