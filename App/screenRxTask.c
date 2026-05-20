#include "screenRxTask.h"

uint8_t screenRxDmaBuf[SCREEN_RX_DMA_SIZE];
QueueHandle_t screenRxQueue;

void ScreenRx_Init(void);
void Screen_ParseRxData(uint8_t *data, uint16_t len);
static int32_t BytesToInt32_LE(const uint8_t b[4]);

void ScreenRx_Task(void *argument)
{
    ScreenRxMsg_t msg;

    while (1)
    {
        if (xQueueReceive(screenRxQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            Screen_ParseRxData(msg.data, msg.len);
        }
    }
}

/**
 * 屏幕接收初始化
 */
void ScreenRx_Init(void)
{
    /*创建screen接收队列*/
    screenRxQueue = xQueueCreate(SCREEN_RX_QUEUE_LEN, sizeof(ScreenRxMsg_t));

    /*启动 DMA + IDLE 接收*/
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3,
                                 screenRxDmaBuf,
                                 SCREEN_RX_DMA_SIZE);
    
    /*关闭半传输中断*/
    __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);

    xTaskCreate(ScreenRx_Task,
                "ScreenRx",
                512,
                NULL,
                3,
                NULL);
}

/**
 * screen 接收数据解析函数
 */
uint8_t ph_temp[4];
uint8_t ph_slope[4];
uint8_t ph_offset[4];
uint8_t ec_alpha[4];
uint8_t ec_cal_gain[4];
uint8_t ec_cal_offset[4];
uint8_t ec_k_cell[4];



void Screen_ParseRxData(uint8_t *data, uint16_t len)
{
    // HAL_UART_Transmit(&huart1, data, len, HAL_MAX_DELAY);

    // 手动添加一个安全的缓冲区，能够使用字符串比较strcmp
    char buf[32];
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, data, len);
    buf[len] = '\0';
    
    // 页面切换
    if (strcmp(buf, "RS485_PARA_PAGE") == 0)
    {
        currentPage = RS485_PARA_PAGE;
        return;
    }
    else if (strcmp(buf, "ADC_PARA_PAGE") == 0)
    {
        currentPage = ADC_PARA_PAGE;
        return;
    }
    else if (strcmp(buf, "PH_CAL_PAGE") == 0)  // 你现在的数据一定进这里！
    {
        currentPage = PH_CAL_PAGE;
        return;
    }
    else if (strcmp(buf, "EC_CAL_PAGE") == 0)
    {
        currentPage = EC_CAL_PAGE;
        return;
    }

    // 校准参数更新
    if (len >= 18 && memcmp(data, "ph_cal", 6) == 0) 
    {
        memcpy(ph_temp, &data[6], 4);
        memcpy(ph_slope, &data[6 + 4], 4);
        memcpy(ph_offset, &data[6 + 8], 4);
        calibration_para.PH_CAL_TEMP_C = (BytesToInt32_LE(ph_temp) / 100.0f);
        calibration_para.PH_SLOPE_25C = (BytesToInt32_LE(ph_slope) / 100.0f);
        calibration_para.PH_OFFSET = (BytesToInt32_LE(ph_offset) / 100.0f);
        AT24CXX_Write(0, (uint8_t *)&calibration_para, sizeof(calibration_para_t));
    }
    if (len >= 22 && memcmp(data, "ec_cal", 6) == 0) 
    {
        memcpy(ec_alpha, &data[6], 4);
        memcpy(ec_cal_gain, &data[6 + 4], 4);
        memcpy(ec_cal_offset, &data[6 + 8], 4);
        memcpy(ec_k_cell, &data[6 + 12], 4);
        calibration_para.EC_ALPHA = (BytesToInt32_LE(ec_alpha) / 100.0f);
        calibration_para.EC_CAL_GAIN = (BytesToInt32_LE(ec_cal_gain) / 100.0f);
        calibration_para.EC_CAL_OFFSET = (BytesToInt32_LE(ec_cal_offset) / 100.0f);
        calibration_para.EC_K_CELL = (BytesToInt32_LE(ec_k_cell) / 100.0f);
        AT24CXX_Write(0, (uint8_t *)&calibration_para, sizeof(calibration_para_t));
    }
}

static int32_t BytesToInt32_LE(const uint8_t b[4])
{
    uint32_t u =
        ((uint32_t)b[0])       |
        ((uint32_t)b[1] << 8)  |
        ((uint32_t)b[2] << 16) |
        ((uint32_t)b[3] << 24);

    return (int32_t)u;
}


