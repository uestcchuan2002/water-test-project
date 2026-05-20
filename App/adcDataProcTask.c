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

/*adc传感器校准参数结构体实例-->开机，需要在EEPROM中读取*/
calibration_para_t calibration_para;

/**
 * adc采集数据处理函数
 */
void adcDataPrcoTask(void)
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

/**
 * 中值滤波
 */
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

/**
 * 滑动平均滤波
 */
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

/**
 * adc采集值转电压值
 */
static float adc_to_voltage(uint16_t adc)
{
    return ((float)adc * 3.3f) / 4095.0f;
}

/**
 * 电压转温度值
 */
static float calc_temperature(float v)
{
    const float Vref = 3.3f;
    const float Rfixed = 10000.0f;
    const float R0 = 10000.0f;
    const float T0 = 298.15f;
    const float B = 3950.0f;

    float Vadc = v;

    if (Vadc <= 0.001f) {
        Vadc = 0.001f;
    } else if (Vadc >= Vref - 0.001f) {
        Vadc = Vref - 0.001f;
    }

    // Vref --- 固定电阻 --- ADC --- NTC --- GND
    float Rntc = Rfixed * Vadc / (Vref - Vadc);

    float T = 1.0f / (1.0f / T0 + logf(Rntc / R0) / B);

    return T - 273.15f;
}

/**
 * 电压转ph值
 */
static float calc_ph(float v_ph, float temperature)
{
    float temp_k = temperature + 273.15f;
    float slope_t = calibration_para.PH_SLOPE_25C * temp_k / 298.15f;

    float ph = slope_t * v_ph + calibration_para.PH_OFFSET;

    if (ph < 0.0f) {
        ph = 0.0f;
    } else if (ph > 14.0f) {
        ph = 14.0f;
    }

    return ph;
}

/**
 * 电压转浊度值
 */
static float calc_turbidity(float v)
{
    float ntu = calibration_para.TURB_A * v * v + calibration_para.TURB_B * v + calibration_para.TURB_C;

    if (ntu < 0.0f) {
        ntu = 0.0f;
    }
    if (ntu > 2000) ntu = ntu -2000;
    return ntu;
}

/**
 * 电压转电导率
 */
static float calc_conductivity(float v, float temperature)
{
    float ec_raw = calibration_para.EC_CAL_GAIN * v + calibration_para.EC_CAL_OFFSET;

    float ec25 = ec_raw / (1.0f + calibration_para.EC_ALPHA * (temperature - 25.0f));

    if (ec25 < 0.0f) {
        ec25 = 0.0f;
    }

    return ec25;
}

