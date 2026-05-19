#include "adcTask.h"

static uint16_t DataBuffer[BATCH_DATA_LEN];
TaskHandle_t xMyAdcTaskHandle = NULL;
QueueHandle_t adcRawQueue = NULL;

void adcTask(void)
{
    adc_raw_frame_t frame;

    xMyAdcTaskHandle = xTaskGetCurrentTaskHandle();
    adcRawQueue = xQueueCreate(2, sizeof(adc_raw_frame_t));

    /* 以DMA的方式启动ADC转换 */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)DataBuffer, BATCH_DATA_LEN);
    // 关闭半传输中断
    __HAL_DMA_DISABLE_IT(hadc1.DMA_Handle, DMA_IT_HT);
    // 开始触发定时器
    HAL_TIM_Base_Start(&htim3);


    for (;;)
    {
        // 等待中断发来的DMA搬运完成信号
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        memcpy(frame.raw, DataBuffer, sizeof(DataBuffer));
        frame.tick = xTaskGetTickCount();

        xQueueSend(adcRawQueue, &frame, pdMS_TO_TICKS(10));
        //LED1_Troggle();
    }
}

