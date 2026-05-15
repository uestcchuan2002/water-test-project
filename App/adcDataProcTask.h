#ifndef __ADCDATAPROCTASK_H
#define __ADCDATAPROCTASK_H

#include "main.h"
#include "adcTask.h"
#include "task.h"
#include "queue.h"

#define MOV_AVG_WIN 4

typedef struct
{
    uint16_t buf[MOV_AVG_WIN];
    uint8_t index;
    uint8_t count;
} moving_avg_t;

extern QueueHandle_t sensorDataQueue;

void addDataPrcoTask(void);

#endif
