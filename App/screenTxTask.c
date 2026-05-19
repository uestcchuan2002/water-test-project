#include "screenTxTask.h"

QueueHandle_t screenTxQueue;
SemaphoreHandle_t screenTxDoneSem;

void ScreenTx_Task(void *argument);

void ScreenTx_Task(void *argument)
{
    ScreenTxMsg_t msg;

    while (1)
    {
        if (xQueueReceive(screenTxQueue, &msg, portMAX_DELAY) == pdPASS)
        {
			/*
			** 1.Take拿信号量
			** 2.启动DMA发送
			** 3.发送成功->等待中断完成后Give
			** 4.发送失败->立刻Give，避免锁死
			*/
            if (xSemaphoreTake(screenTxDoneSem,  portMAX_DELAY) == pdPASS)
            {
                if (HAL_UART_Transmit_DMA(&huart3, msg.data, msg.len) != HAL_OK)
                {
                    xSemaphoreGive(screenTxDoneSem);
                }
            }
        }
    }
}

void ScreenTx_Init(void)
{
    screenTxQueue = xQueueCreate(SCREEN_TX_QUEUE_LEN, sizeof(ScreenTxMsg_t));
    screenTxDoneSem = xSemaphoreCreateBinary();
	
	/* 第一次释放信号量 */
    xSemaphoreGive(screenTxDoneSem);

    xTaskCreate(ScreenTx_Task,
                "ScreenTx",
                512,
                NULL,
                33,
                NULL);
}

void Screen_SendCmd(const char *cmd)
{
    ScreenTxMsg_t msg;
    uint16_t len = strlen(cmd);

    if (len + 3 > SCREEN_TX_MAX_LEN)
        return;

    memcpy(msg.data, cmd, len);

    msg.data[len++] = 0xFF;
    msg.data[len++] = 0xFF;
    msg.data[len++] = 0xFF;

    msg.len = len;

    xQueueSend(screenTxQueue, &msg, pdMS_TO_TICKS(20));
}

