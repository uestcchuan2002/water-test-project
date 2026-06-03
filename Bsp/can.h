#ifndef __CAN_H
#define __CAN_H

#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>

#define CAN1_RX_QUEUE_LEN       16U
#define CAN1_RX_WAIT_FOREVER    0xFFFFFFFFUL

typedef struct
{
    uint32_t std_id;
    uint32_t ide;
    uint32_t rtr;
    uint8_t dlc;
    uint8_t data[8];
    uint32_t tick;
} CanFrame_t;

extern CAN_HandleTypeDef CAN1_Handler;
extern QueueHandle_t can1RxQueue;

void CAN1_RxQueue_Init(void);
uint8_t CAN1_Mode_Init(uint32_t tsjw, uint32_t tbs2, uint32_t tbs1, uint32_t brp, uint32_t mode);
uint8_t CAN1_Config(void);
void CAN_Config(void);

uint8_t CAN1_Send_Frame(uint32_t std_id, const uint8_t *data, uint8_t len, uint32_t timeout_ms);
uint8_t CAN1_Receive_Frame(CanFrame_t *frame, uint32_t timeout_ms);

uint8_t CAN1_Send_Msg(uint8_t *msg, uint8_t len);
uint8_t CAN1_Receive_Msg(uint8_t *buf);

uint32_t CAN1_GetRxOverflowCount(void);
uint32_t CAN1_GetLastErrorCode(void);

#endif
