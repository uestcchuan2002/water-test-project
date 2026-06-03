#ifndef __CANTXTASK_H
#define __CANTXTASK_H

#include "cmsis_os.h"
#include "task.h"
#include "can.h"
#include "canSensorProtocol.h"
#include "lcd.h"
#include "usart.h"
#include <stdio.h>

#ifndef CAN_SENSOR_CAN_MODE
/* 回环测试 */
// #define CAN_SENSOR_CAN_MODE CAN_MODE_LOOPBACK
/* 正常工作模式 */
#define CAN_SENSOR_CAN_MODE CAN_MODE_NORMAL
#endif

void CanTxTask_Init(void);
void CanTxTask_Run(void *argument);
void CanTxTask_SetPollPeriodMs(uint32_t poll_period_ms);
uint32_t CanTxTask_GetPollPeriodMs(void);

#endif
