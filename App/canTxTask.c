#include "canTxTask.h"

static volatile uint32_t canTxPollPeriodMs = CAN_SENSOR_DEFAULT_POLL_MS;
static uint8_t canRequestSequence = 0U;

/**
 * @brief 初始化CAN发送任务相关资源。
 *
 * 该函数用于初始化与CAN发送任务相关的底层组件。当前实现中，
 * 主要调用CAN1接收队列的初始化函数，为后续CAN通信做准备。
 *
 * @note 本函数不接受任何参数，也不返回任何值。
 */
void CanTxTask_Init(void)
{
    CAN1_RxQueue_Init();
}

/**
 * @brief 设置CAN发送任务的轮询周期（单位：毫秒）。
 *
 * 该函数用于配置CAN发送任务的轮询间隔时间。若传入的轮询周期小于系统定义的最小允许值
 * CAN_SENSOR_MIN_POLL_MS，则自动将其限制为该最小值，以确保系统稳定性和实时性。
 *
 * @param[in] poll_period_ms 要设置的轮询周期，单位为毫秒。如果该值小于CAN_SENSOR_MIN_POLL_MS，
 *                           则实际设置的值将被限制为CAN_SENSOR_MIN_POLL_MS。
 */
void CanTxTask_SetPollPeriodMs(uint32_t poll_period_ms)
{
    if (poll_period_ms < CAN_SENSOR_MIN_POLL_MS)
    {
        poll_period_ms = CAN_SENSOR_MIN_POLL_MS;
    }

    canTxPollPeriodMs = poll_period_ms;
}

/**
 * @brief 获取CAN发送任务的轮询周期（单位：毫秒）。
 *
 * @return uint32_t 当前配置的CAN发送任务轮询周期，单位为毫秒。
 */
uint32_t CanTxTask_GetPollPeriodMs(void)
{
    return canTxPollPeriodMs;
}

void CanTxTask_Run(void *argument)
{
    const CanSensorDescriptor_t *sensorTable;
    uint8_t sensorCount = 0U;
    TickType_t lastWakeTime;
    uint8_t initStatus;

    (void)argument;

    CanTxTask_Init();

    initStatus = CAN1_Mode_Init(CAN_SJW_1TQ,
                                CAN_BS2_6TQ,
                                CAN_BS1_7TQ,
                                6U,
                                CAN_SENSOR_CAN_MODE);
    configASSERT(initStatus == 0U);
    configASSERT(CAN1_Config() == 0U);
    
    /**
     * 获取当前的相关信息
     * sensorTable:获取到的can通信协议表
     * sensorCount:获取到的当前传感器数量
     */
    sensorTable = CanSensorProtocol_GetTable(&sensorCount);
    lastWakeTime = xTaskGetTickCount();

    printf("can tx task start, poll = %lu ms\r\n", (unsigned long)CanTxTask_GetPollPeriodMs());

    for (;;)
    {
        uint32_t pollMs = CanTxTask_GetPollPeriodMs();

        for (uint8_t i = 0U; i < sensorCount; i++)
        {
            uint8_t txData[CAN_SENSOR_REQUEST_DLC];
            uint8_t txStatus;

            if (CanSensorProtocol_BuildReadRequest(&sensorTable[i],
                                                   canRequestSequence++,
                                                   txData,
                                                   sizeof(txData)) != 0U)
            {
                continue;
            }

            txStatus = CAN1_Send_Frame(sensorTable[i].request_id,
                                       txData,
                                       CAN_SENSOR_REQUEST_DLC,
                                       CAN_SENSOR_TX_TIMEOUT_MS);
            if (txStatus != 0U)
            {
                printf("can tx fail id = 0x%03lX status = %u err = 0x%08lX\r\n",
                       (unsigned long)sensorTable[i].request_id,
                       txStatus,
                       (unsigned long)CAN1_GetLastErrorCode());
            } 
            
            osDelay(CAN_SENSOR_INTER_FRAME_MS);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(pollMs));
    }
}
