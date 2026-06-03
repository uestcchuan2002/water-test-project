#include "canRxTask.h"

QueueHandle_t canSensorDataQueue = NULL;

static CanSensorSample_t canLatestSamples[CAN_SENSOR_COUNT];
static uint8_t canLatestValid[CAN_SENSOR_COUNT];

static uint8_t CanRxTask_IndexByType(CanSensorType_t type);
static void CanRxTask_UpdateLatest(const CanSensorSample_t *sample);
static void CanRxTask_PublishSample(const CanSensorSample_t *sample);

void CanRxTask_Init(void)
{
    CAN1_RxQueue_Init();

    if (canSensorDataQueue == NULL)
    {
        canSensorDataQueue = xQueueCreate(CAN_SENSOR_DATA_QUEUE_LEN, sizeof(CanSensorSample_t));
        configASSERT(canSensorDataQueue != NULL);
    }
}

void CanRxTask_Run(void *argument)
{
    CanFrame_t frame;

    (void)argument;

    CanRxTask_Init();

    for (;;)
    {
        if (CAN1_Receive_Frame(&frame, CAN1_RX_WAIT_FOREVER) == 0U)
        {
            CanSensorSample_t sample;
            CanSensorParseStatus_t status;

            // 解析CAN传感器协议的响应帧，并填充采样数据结构
            status = CanSensorProtocol_ParseResponse(&frame, &sample);

            if ((status == CAN_SENSOR_PARSE_OK) ||
                (status == CAN_SENSOR_PARSE_ERROR_STATUS))
            {
                CanRxTask_UpdateLatest(&sample);
                CanRxTask_PublishSample(&sample);

                printf("--can rx %s=%.3f mg/L status=%u seq=%u\r\n",
                       sample.name,
                       sample.value,
                       sample.status,
                       sample.sequence);
            }
            else if (status != CAN_SENSOR_PARSE_UNKNOWN_ID)
            {
                printf("can parse fail id=0x%03lX status=%s\r\n",
                       (unsigned long)frame.std_id,
                       CanSensorProtocol_ParseStatusText(status));
            }
        }
    }
}

uint8_t CanRxTask_GetLatest(CanSensorType_t type, CanSensorSample_t *sample)
{
    uint8_t index;

    if (sample == NULL)
    {
        return 0U;
    }

    index = CanRxTask_IndexByType(type);
    if (index >= CAN_SENSOR_COUNT)
    {
        return 0U;
    }

    taskENTER_CRITICAL();
    if (canLatestValid[index] == 0U)
    {
        taskEXIT_CRITICAL();
        return 0U;
    }

    *sample = canLatestSamples[index];
    taskEXIT_CRITICAL();

    return 1U;
}

static uint8_t CanRxTask_IndexByType(CanSensorType_t type)
{
    if ((type < CAN_SENSOR_FREE_CHLORINE) || (type > CAN_SENSOR_COPPER_ION))
    {
        return CAN_SENSOR_COUNT;
    }

    return (uint8_t)((uint8_t)type - 1U);
}

/**
 * @brief 更新指定类型的CAN传感器最新采样数据。
 *
 * 该函数根据传入的CAN传感器采样数据，将其存储到全局最新样本数组中，并标记为有效。
 * 使用临界区保护以确保多任务环境下的数据一致性。
 *
 * @param[in] sample 指向CAN传感器采样数据的指针。若为NULL，则函数直接返回。
 * @return 无（void）
 */
static void CanRxTask_UpdateLatest(const CanSensorSample_t *sample)
{
    uint8_t index;

    if (sample == NULL)
    {
        return;
    }

    index = CanRxTask_IndexByType(sample->type);
    if (index >= CAN_SENSOR_COUNT)
    {
        return;
    }

    /* 进入临界区，安全地更新最新采样数据和有效性标志 */
    taskENTER_CRITICAL();
    canLatestSamples[index] = *sample;
    canLatestValid[index] = 1U;
    taskEXIT_CRITICAL();
}

/**
 * @brief 将CAN传感器采样数据发布到队列中。
 *
 * 该函数尝试将传入的CAN传感器采样数据发送到全局队列canSensorDataQueue。
 * 如果队列已满，则先丢弃队列中最旧的一个样本，再尝试重新发送当前样本。
 * 此策略确保队列始终保留最新的采样数据。
 *
 * @param[in] sample 指向要发布的CAN传感器采样数据的指针。若为NULL，则函数直接返回。
 * @return void 无返回值。
 */
static void CanRxTask_PublishSample(const CanSensorSample_t *sample)
{
    if ((sample == NULL) || (canSensorDataQueue == NULL))
    {
        return;
    }

    /* 尝试将样本发送到队列；若队列已满，则丢弃最旧样本后重试 */
    if (xQueueSend(canSensorDataQueue, sample, 0U) != pdPASS)
    {
        CanSensorSample_t dropped;

        (void)xQueueReceive(canSensorDataQueue, &dropped, 0U);
        (void)xQueueSend(canSensorDataQueue, sample, 0U);
    }
}
