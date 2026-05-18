#ifndef __ADCDATAPROCTASK_H
#define __ADCDATAPROCTASK_H

#include "main.h"
#include "adcTask.h"
#include "task.h"
#include "queue.h"
#include <math.h>

#define MOV_AVG_WIN 4

#define PH_CAL_TEMP_C      25.0f
#define PH_SLOPE_25C      (-5.70f)   // pH/V，示例值，需要标定
#define PH_OFFSET         21.34f     // 示例值，需要标定

#define TURB_A            (-1120.4f) // 示例曲线参数
#define TURB_B            (5742.3f)
#define TURB_C            (-4352.9f)

#define EC_K_CELL         1.0f       // 电极常数 K，cm^-1
#define EC_ALPHA          0.02f      // 温度补偿系数，约 2%/°C
#define EC_CAL_GAIN       1.0f       // 电导率标定增益
#define EC_CAL_OFFSET     0.0f       // 电导率标定偏移

typedef struct
{
    uint16_t buf[MOV_AVG_WIN];
    uint8_t index;
    uint8_t count;
} moving_avg_t;

extern QueueHandle_t sensorDataQueue;

void adcDataPrcoTask(void);

#endif
