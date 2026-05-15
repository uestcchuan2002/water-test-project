#ifndef __ADCTASK_H
#define __ADCTASK_H

#include "main.h"
#include "adc.h"
#include "tim.h"
#include "led.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <queue.h>

#define ADC_CH_NUM              4
#define ADC_SAMPLE_GROUPS       10
#define BATCH_DATA_LEN          ADC_CH_NUM * ADC_SAMPLE_GROUPS

extern uint16_t DataBuffer[BATCH_DATA_LEN];
extern TaskHandle_t xMyAdcTaskHandle;
extern QueueHandle_t adcRawQueue;

typedef enum
{
    ADC_CH_PH = 0,
    ADC_CH_TEMP,
    ADC_CH_TURBIDITY,
    ADC_CH_EC,
} adc_channel_id_t;

typedef struct
{
    uint16_t raw[BATCH_DATA_LEN];   // 原始数据
    uint32_t tick;                  // 时间戳
} adc_raw_frame_t;

typedef struct
{
    uint16_t adc_raw[ADC_CH_NUM];

    float voltage[ADC_CH_NUM];

    float ph;
    float temperature;
    float turbidity;
    float conductivity;

    uint16_t status;
    uint32_t tick;
} sensor_data_t;

void adcTask(void);

#endif
