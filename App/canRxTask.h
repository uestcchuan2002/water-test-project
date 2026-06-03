#ifndef __CANRXTASK_H
#define __CANRXTASK_H

#include "cmsis_os.h"
#include "task.h"
#include "queue.h"
#include "can.h"
#include "canSensorProtocol.h"
#include <stdio.h>

#define CAN_SENSOR_DATA_QUEUE_LEN 8U

extern QueueHandle_t canSensorDataQueue;

void CanRxTask_Init(void);
void CanRxTask_Run(void *argument);
uint8_t CanRxTask_GetLatest(CanSensorType_t type, CanSensorSample_t *sample);

#endif
