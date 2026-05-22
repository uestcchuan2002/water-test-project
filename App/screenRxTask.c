#include "screenRxTask.h"
#include "24cxx.h"
#include "adcDataProcTask.h"
#include "displayTask.h"
#include "usart.h"
#include <string.h>

uint8_t screenRxDmaBuf[SCREEN_RX_DMA_SIZE];
QueueHandle_t screenRxQueue = NULL;

static void Screen_ParseRxData(uint8_t *data, uint16_t len);
static int32_t BytesToInt32_LE(const uint8_t b[4]);
static float Screen_ReadFixed100_LE(const uint8_t b[4]);

void ScreenRx_Init(void)
{
    if (screenRxQueue == NULL)
    {
        screenRxQueue = xQueueCreate(SCREEN_RX_QUEUE_LEN, sizeof(ScreenRxMsg_t));
        configASSERT(screenRxQueue != NULL);
    }

    configASSERT(HAL_UARTEx_ReceiveToIdle_DMA(&huart3,
                                              screenRxDmaBuf,
                                              SCREEN_RX_DMA_SIZE) == HAL_OK);
    __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
}

void ScreenRx_Task(void *argument)
{
    ScreenRxMsg_t msg;

    (void)argument;
    configASSERT(screenRxQueue != NULL);

    for (;;)
    {
        if (xQueueReceive(screenRxQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            Screen_ParseRxData(msg.data, msg.len);
        }
    }
}

static void Screen_ParseRxData(uint8_t *data, uint16_t len)
{
    char buf[32];

    if (len >= sizeof(buf))
    {
        len = sizeof(buf) - 1U;
    }

    memcpy(buf, data, len);
    buf[len] = '\0';

    if (strcmp(buf, "RS485_PARA_PAGE") == 0)
    {
        DisplayTask_SetPage(RS485_PARA_PAGE);
        return;
    }
    else if (strcmp(buf, "ADC_PARA_PAGE") == 0)
    {
        DisplayTask_SetPage(ADC_PARA_PAGE);
        return;
    }
    else if (strcmp(buf, "PH_CAL_PAGE") == 0)
    {
        DisplayTask_SetPage(PH_CAL_PAGE);
        return;
    }
    else if (strcmp(buf, "EC_CAL_PAGE") == 0)
    {
        DisplayTask_SetPage(EC_CAL_PAGE);
        return;
    }
    else if (strcmp(buf, "STORAGE_PAGE") == 0)
    {
        DisplayTask_SetPage(STORAGE_PAGE);
        return;
    }

    if (len >= 18U && memcmp(data, "ph_cal", 6U) == 0)
    {
        calibration_para.PH_CAL_TEMP_C = Screen_ReadFixed100_LE(&data[6]);
        calibration_para.PH_SLOPE_25C = Screen_ReadFixed100_LE(&data[6 + 4]);
        calibration_para.PH_OFFSET = Screen_ReadFixed100_LE(&data[6 + 8]);
        AT24CXX_Write(0, (uint8_t *)&calibration_para, sizeof(calibration_para_t));
    }
    else if (len >= 22U && memcmp(data, "ec_cal", 6U) == 0)
    {
        calibration_para.EC_ALPHA = Screen_ReadFixed100_LE(&data[6]);
        calibration_para.EC_CAL_GAIN = Screen_ReadFixed100_LE(&data[6 + 4]);
        calibration_para.EC_CAL_OFFSET = Screen_ReadFixed100_LE(&data[6 + 8]);
        calibration_para.EC_K_CELL = Screen_ReadFixed100_LE(&data[6 + 12]);
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

static float Screen_ReadFixed100_LE(const uint8_t b[4])
{
    return (float)BytesToInt32_LE(b) / 100.0f;
}
