#include "filter.h"

static moving_avg_t adc_mov_avg[ADC_CH_NUM];

/* adc中值滤波算法 */
static uint16_t adc_median_filter(uint16_t *buf, uint8_t ch)
{
    uint16_t temp[ADC_SAMPLE_GROUPS];

    for (uint8_t i = 0; i < ADC_SAMPLE_GROUPS; i++)
    {
        temp[i] = buf[i * ADC_CH_NUM + ch];
    }

    for (uint8_t i = 0; i < ADC_SAMPLE_GROUPS - 1; i++)
    {
        for (uint8_t j = 0; j < ADC_SAMPLE_GROUPS - 1 - i; j++)
        {
            if (temp[j] > temp[j + 1])
            {
                uint16_t t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }

    return (temp[4] + temp[5]) / 2;
}

/* 对连续4次中值滤波的数据做滑动平均处理 */
static uint16_t moving_average_update(moving_avg_t *avg, uint16_t input)
{
    uint32_t sum = 0;

    avg->buf[avg->index] = input;
    avg->index = (avg->index + 1) % AVG_WIN;

    if (avg->count < AVG_WIN)
    {
        avg->count++;
    }

    for (uint8_t i = 0; i < avg->count; i++)
    {
        sum += avg->buf[i];
    }

    return (uint16_t)(sum / avg->count);
}




