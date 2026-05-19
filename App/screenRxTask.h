#ifndef __SCREENRXTASK_H
#define __SCREENRXTASK_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

#define SCREEN_RX_DMA_SIZE     128
#define SCREEN_RX_QUEUE_LEN    8

extern uint8_t screenRxDmaBuf[SCREEN_RX_DMA_SIZE];
extern QueueHandle_t screenRxQueue;

typedef struct
{
    uint8_t data[SCREEN_RX_DMA_SIZE];
    uint16_t len;
} ScreenRxMsg_t;

void ScreenRx_Init(void);

#endif

