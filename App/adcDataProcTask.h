#ifndef __ADCDATAPROCTASK_H
#define __ADCDATAPROCTASK_H

#include "main.h"
#include "adcTask.h"
#include "task.h"
#include "queue.h"
#include <math.h>
#include "storageTask.h"

#define MOV_AVG_WIN 4



/**
 * adc传感器校准系数...
 */
typedef struct 
{
    float PH_CAL_TEMP_C;        // pH校准基准温度(摄氏度)
    float PH_SLOPE_25C;         // 25℃下pH电压斜率，单位：pH/伏
    float PH_OFFSET;            // pH电压偏移量
    float TURB_A;               // 三点校准值A
    float TURB_B;               // 三点校准值B
    float TURB_C;               // 三点校准值C
    float EC_K_CELL;            // 电极常数 K，cm^-1
    float EC_ALPHA;             // 温度补偿系数，约 2%/°C
    float EC_CAL_GAIN;          // 电导率标定增益
    float EC_CAL_OFFSET;        // 电导率标定偏移
} calibration_para_t;


typedef struct
{
    uint16_t buf[MOV_AVG_WIN];
    uint8_t index;
    uint8_t count;
} moving_avg_t;

#define SENSOR_DATA_QUEUE_LEN 1U

extern QueueHandle_t sensorDataQueue;
extern calibration_para_t calibration_para;

void AdcDataProcTask_Init(void);
void AdcDataProcTask_Run(void *argument);
void adcDataPrcoTask(void);

#endif
