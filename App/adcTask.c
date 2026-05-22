#include "adcTask.h"

static uint16_t DataBuffer[BATCH_DATA_LEN];
TaskHandle_t xMyAdcTaskHandle = NULL;
QueueHandle_t adcRawQueue = NULL;

void AdcTask_Init(void)
{
    if (adcRawQueue == NULL)
    {
        adcRawQueue = xQueueCreate(ADC_RAW_QUEUE_LEN, sizeof(adc_raw_frame_t));
        configASSERT(adcRawQueue != NULL);
    }
}

void AdcTask_Run(void *argument)
{
    adc_raw_frame_t frame;

    (void)argument;

    AdcTask_Init();
    xMyAdcTaskHandle = xTaskGetCurrentTaskHandle();

    configASSERT(HAL_ADC_Start_DMA(&hadc1, (uint32_t *)DataBuffer, BATCH_DATA_LEN) == HAL_OK);
    __HAL_DMA_DISABLE_IT(hadc1.DMA_Handle, DMA_IT_HT);
    configASSERT(HAL_TIM_Base_Start(&htim3) == HAL_OK);

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        memcpy(frame.raw, DataBuffer, sizeof(DataBuffer));
        frame.tick = xTaskGetTickCount();

        xQueueSend(adcRawQueue, &frame, pdMS_TO_TICKS(10));
    }
}

void adcTask(void)
{
    AdcTask_Run(NULL);
}
