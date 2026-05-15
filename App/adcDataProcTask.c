#include "adcDataProcTask.h"

QueueHandle_t sensorDataQueue;
static moving_avg_t adcMovAvg[ADC_CH_NUM];

static uint16_t adc_median_filter(uint16_t *buf, uint8_t ch);
static uint16_t moving_average_update(moving_avg_t *avg, uint16_t input);

static float adc_to_voltage(uint16_t adc);
static float calc_temperature(float v);
static float calc_ph(float v_ph, float temperature);
static float calc_turbidity(float v);
static float calc_conductivity(float v, float temperature);


void addDataPrcoTask(void)
{
    adc_raw_frame_t raw_frame;
    sensor_data_t sensor_data;

    sensorDataQueue = xQueueCreate(1, sizeof(sensor_data_t));

    while (1)
    {
        if (xQueueReceive(adcRawQueue, &raw_frame, portMAX_DELAY) == pdPASS)
        {
            for (uint8_t ch = 0; ch < ADC_CH_NUM; ch++)
            {
                uint16_t one_sec_value;

                one_sec_value = adc_median_filter(raw_frame.raw, ch);
                sensor_data.adc_raw[ch] = moving_average_update(&adcMovAvg[ch], one_sec_value);
                sensor_data.voltage[ch] = adc_to_voltage(sensor_data.adc_raw[ch]);
            }

            sensor_data.temperature = calc_temperature(sensor_data.voltage[ADC_CH_TEMP]);
            sensor_data.ph = calc_ph(sensor_data.voltage[ADC_CH_PH],
                                     sensor_data.temperature);
            sensor_data.turbidity = calc_turbidity(sensor_data.voltage[ADC_CH_TURBIDITY]);
            sensor_data.conductivity = calc_conductivity(sensor_data.voltage[ADC_CH_EC],
                                                         sensor_data.temperature);

            sensor_data.tick = raw_frame.tick;
            sensor_data.status = 0;

            xQueueOverwrite(sensorDataQueue, &sensor_data);
        }
    }
}

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

static uint16_t moving_average_update(moving_avg_t *avg, uint16_t input)
{
    uint32_t sum = 0;

    avg->buf[avg->index] = input;
    avg->index = (avg->index + 1) % MOV_AVG_WIN;

    if (avg->count < MOV_AVG_WIN)
    {
        avg->count++;
    }

    for (uint8_t i = 0; i < avg->count; i++)
    {
        sum += avg->buf[i];
    }

    return (uint16_t)(sum / avg->count);
}

static float adc_to_voltage(uint16_t adc)
{
    return ((float)adc * 3.3f) / 4095.0f;
}

static float calc_temperature(float v)
{
    /* TODO: 根据NTC/PT100/温度芯片公式换算 */
    return v;
}

static float calc_ph(float v_ph, float temperature)
{
    /* TODO: 根据pH标定斜率、截距和温度补偿换算 */
    return v_ph;
}

static float calc_turbidity(float v)
{
    /* TODO: 根据浊度传感器曲线换算 */
    return v;
}

static float calc_conductivity(float v, float temperature)
{
    /* TODO: 根据电导率传感器K值和温度补偿换算 */
    return v;
}

