#include "screenTxTask.h"

QueueHandle_t screenTxQueue = NULL;
SemaphoreHandle_t screenTxDoneSem = NULL;

void ScreenTx_Init(void)
{
    if (screenTxQueue == NULL)
    {
        screenTxQueue = xQueueCreate(SCREEN_TX_QUEUE_LEN, sizeof(ScreenTxMsg_t));
        configASSERT(screenTxQueue != NULL);
    }

    if (screenTxDoneSem == NULL)
    {
        screenTxDoneSem = xSemaphoreCreateBinary();
        configASSERT(screenTxDoneSem != NULL);
        xSemaphoreGive(screenTxDoneSem);
    }
}

void ScreenTx_Task(void *argument)
{
    ScreenTxMsg_t msg;

    (void)argument;
    configASSERT(screenTxQueue != NULL);
    configASSERT(screenTxDoneSem != NULL);

    for (;;)
    {
        if (xQueueReceive(screenTxQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            if (xSemaphoreTake(screenTxDoneSem, portMAX_DELAY) == pdPASS)
            {
                if (HAL_UART_Transmit_DMA(&huart3, msg.data, msg.len) != HAL_OK)
                {
                    xSemaphoreGive(screenTxDoneSem);
                }
            }
        }
    }
}

void Screen_SendCmd(const char *cmd)
{
    ScreenTxMsg_t msg;
    size_t len;

    if ((screenTxQueue == NULL) || (cmd == NULL))
    {
        return;
    }

    len = strlen(cmd);
    if (len + 3U > SCREEN_TX_MAX_LEN)
    {
        return;
    }

    memcpy(msg.data, cmd, len);

    msg.data[len++] = 0xFF;
    msg.data[len++] = 0xFF;
    msg.data[len++] = 0xFF;
    msg.len = (uint16_t)len;

    xQueueSend(screenTxQueue, &msg, pdMS_TO_TICKS(20));
}
