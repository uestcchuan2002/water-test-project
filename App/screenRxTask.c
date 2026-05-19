#include "screenRxTask.h"

uint8_t screenRxDmaBuf[SCREEN_RX_DMA_SIZE];
QueueHandle_t screenRxQueue;

void ScreenRx_Init(void);
void Screen_ParseRxData(uint8_t *data, uint16_t len);

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
void Screen_ParseRxData(uint8_t *data, uint16_t len)
{
    // 这里先根据你的串口屏协议解析
    // 例如判断按键事件、页面切换、参数设置等
    HAL_UART_Transmit(&huart1, data, len, HAL_MAX_DELAY);
}
