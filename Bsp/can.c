#include "can.h"
#include <string.h>

CAN_HandleTypeDef CAN1_Handler;
QueueHandle_t can1RxQueue = NULL;

static volatile uint32_t can1RxOverflowCount = 0U;
static volatile uint32_t can1LastErrorCode = 0U;

/**
 * @brief 初始化 CAN1 接收消息队列。
 *
 * 该函数用于创建一个 FreeRTOS 队列，用于存储从 CAN1 接口接收到的帧数据。
 * 若队列尚未创建（即 can1RxQueue 为 NULL），则调用 xQueueCreate 创建指定长度和元素大小的队列，
 * 并通过 configASSERT 确保队列创建成功。若队列已存在，则不执行任何操作。
 *
 * @note 本函数无参数，亦无返回值。
 */
void CAN1_RxQueue_Init(void)
{
    if (can1RxQueue == NULL)
    {
        can1RxQueue = xQueueCreate(CAN1_RX_QUEUE_LEN, sizeof(CanFrame_t));
        configASSERT(can1RxQueue != NULL);
    }
}


/**
 * @brief 初始化CAN1外设的工作模式和波特率时序参数。
 *
 * 该函数配置CAN1控制器的初始化参数，包括波特率分频器、同步跳转宽度、时间段1/2等，
 * 并调用HAL库完成底层硬件初始化。同时会初始化CAN接收队列。
 *
 * @param[in] tsjw 同步跳转宽度（Time Segment Jump Width），取值范围通常为1~4，单位为时间量子（TQ）。
 * @param[in] tbs2 时间段2（Time Segment 2），即相位缓冲段2，取值范围通常为1~8，单位为TQ。
 * @param[in] tbs1 时间段1（Time Segment 1），即传播段+相位缓冲段1，取值范围通常为1~16，单位为TQ。
 * @param[in] brp 波特率预分频器（Baud Rate Prescaler），用于设置CAN位时间的基本时钟分频，取值范围通常为1~1024。
 * @param[in] mode CAN工作模式，如正常模式（CAN_MODE_NORMAL）、回环模式（CAN_MODE_LOOPBACK）等，由HAL库定义。
 *
 * @return uint8_t 初始化结果：
 *         - 0U：初始化成功；
 *         - 1U：初始化失败，错误码已存入全局变量 can1LastErrorCode。
 */
uint8_t CAN1_Mode_Init(uint32_t tsjw, uint32_t tbs2, uint32_t tbs1, uint32_t brp, uint32_t mode)
{
    CAN1_RxQueue_Init();

    memset(&CAN1_Handler, 0, sizeof(CAN1_Handler));
    CAN1_Handler.Instance = CAN1;
    CAN1_Handler.Init.Prescaler = brp;
    CAN1_Handler.Init.Mode = mode;
    CAN1_Handler.Init.SyncJumpWidth = tsjw;
    CAN1_Handler.Init.TimeSeg1 = tbs1;
    CAN1_Handler.Init.TimeSeg2 = tbs2;
    CAN1_Handler.Init.TimeTriggeredMode = DISABLE;
    CAN1_Handler.Init.AutoBusOff = ENABLE;
    CAN1_Handler.Init.AutoWakeUp = DISABLE;
    CAN1_Handler.Init.AutoRetransmission = ENABLE;
    CAN1_Handler.Init.ReceiveFifoLocked = DISABLE;
    CAN1_Handler.Init.TransmitFifoPriority = DISABLE;

    if (HAL_CAN_Init(&CAN1_Handler) != HAL_OK)
    {
        can1LastErrorCode = HAL_CAN_GetError(&CAN1_Handler);
        return 1U;
    }

    return 0U;
}


void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
    GPIO_InitTypeDef GPIO_Initure;

    if (hcan->Instance != CAN1)
    {
        return;
    }

    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_Initure.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Initure.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);

    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance != CAN1)
    {
        return;
    }

    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
    __HAL_RCC_CAN1_CLK_DISABLE();
}

uint8_t CAN1_Config(void)
{
    CAN_FilterTypeDef sFilterConfig;

    CAN1_RxQueue_Init();

    memset(&sFilterConfig, 0, sizeof(sFilterConfig));
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&CAN1_Handler, &sFilterConfig) != HAL_OK)
    {
        can1LastErrorCode = HAL_CAN_GetError(&CAN1_Handler);
        return 1U;
    }

    if (HAL_CAN_Start(&CAN1_Handler) != HAL_OK)
    {
        can1LastErrorCode = HAL_CAN_GetError(&CAN1_Handler);
        return 2U;
    }

    if (HAL_CAN_ActivateNotification(&CAN1_Handler,
                                     CAN_IT_RX_FIFO0_MSG_PENDING |
                                     CAN_IT_RX_FIFO0_FULL |
                                     CAN_IT_RX_FIFO0_OVERRUN |
                                     CAN_IT_ERROR_WARNING |
                                     CAN_IT_ERROR_PASSIVE |
                                     CAN_IT_BUSOFF |
                                     CAN_IT_LAST_ERROR_CODE |
                                     CAN_IT_ERROR) != HAL_OK)
    {
        can1LastErrorCode = HAL_CAN_GetError(&CAN1_Handler);
        return 3U;
    }

    return 0U;
}

void CAN_Config(void)
{
    configASSERT(CAN1_Config() == 0U);
}

/**
 * @brief 通过CAN1外设发送一个标准帧数据
 *
 * 此函数用于通过STM32的CAN1外设发送一个标准标识符（11位）的数据帧。
 * 函数会等待可用的发送邮箱，并在超时或发生错误时返回相应的错误码。
 *
 * @param[in] std_id      标准CAN标识符（11位），有效范围为0x000 ~ 0x7FF
 * @param[in] data        指向要发送的数据缓冲区的指针；当len > 0时，该指针不得为NULL
 * @param[in] len         要发送的数据长度（字节数），有效范围为0 ~ 8
 * @param[in] timeout_ms  发送等待超时时间（毫秒）；若为0，则表示不等待（立即返回）
 *
 * @return uint8_t        函数执行结果状态码：
 *                        - 0U: 发送成功
 *                        - 1U: 参数错误（ID超出范围、长度非法或data指针无效）
 *                        - 2U: 等待发送邮箱超时
 *                        - 3U: HAL库底层发送失败，错误码已保存至全局变量can1LastErrorCode
 */
uint8_t CAN1_Send_Frame(uint32_t std_id, const uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;
    uint8_t txData[8] = {0};
    uint32_t start;

    /* 参数合法性校验：检查数据长度、数据指针和标准ID范围 */
    if ((len > 8U) || ((len > 0U) && (data == NULL)) || (std_id > 0x7FFU))
    {
        return 1U;
    }

    memcpy(txData, data, len);

    /* 等待有空闲的发送邮箱，支持超时机制 */
    start = HAL_GetTick();
    while (HAL_CAN_GetTxMailboxesFreeLevel(&CAN1_Handler) == 0U)
    {
        if ((timeout_ms == 0U) || ((HAL_GetTick() - start) >= timeout_ms))
        {
            return 2U;
        }
    }

    /* 配置CAN发送帧头参数，使用标准帧格式 */
    memset(&txHeader, 0, sizeof(txHeader));
    txHeader.StdId = std_id;
    txHeader.ExtId = 0U;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = len;
    txHeader.TransmitGlobalTime = DISABLE;

    /* 调用HAL库将消息加入发送队列 */
    if (HAL_CAN_AddTxMessage(&CAN1_Handler, &txHeader, txData, &txMailbox) != HAL_OK)
    {
        can1LastErrorCode = HAL_CAN_GetError(&CAN1_Handler);
        return 3U;
    }

    return 0U;
}

uint8_t CAN1_Receive_Frame(CanFrame_t *frame, uint32_t timeout_ms)
{
    TickType_t waitTicks;

    if ((frame == NULL) || (can1RxQueue == NULL))
    {
        return 1U;
    }

    waitTicks = (timeout_ms == CAN1_RX_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);

    if (xQueueReceive(can1RxQueue, frame, waitTicks) != pdPASS)
    {
        return 2U;
    }

    return 0U;
}

uint8_t CAN1_Send_Msg(uint8_t *msg, uint8_t len)
{
    return CAN1_Send_Frame(0x12U, msg, len, 10U);
}

uint8_t CAN1_Receive_Msg(uint8_t *buf)
{
    CanFrame_t frame;

    if (buf == NULL)
    {
        return 0U;
    }

    if (CAN1_Receive_Frame(&frame, 0U) != 0U)
    {
        return 0U;
    }

    memcpy(buf, frame.data, frame.dlc);
    return frame.dlc;
}

uint32_t CAN1_GetRxOverflowCount(void)
{
    return can1RxOverflowCount;
}

uint32_t CAN1_GetLastErrorCode(void)
{
    return can1LastErrorCode;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hcan->Instance != CAN1)
    {
        return;
    }

    while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0U)
    {
        CAN_RxHeaderTypeDef rxHeader;
        CanFrame_t frame;
        uint8_t rxData[8] = {0};

        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK)
        {
            can1LastErrorCode = HAL_CAN_GetError(hcan);
            break;
        }

        frame.std_id = (rxHeader.IDE == CAN_ID_STD) ? rxHeader.StdId : rxHeader.ExtId;
        frame.ide = rxHeader.IDE;
        frame.rtr = rxHeader.RTR;
        frame.dlc = rxHeader.DLC;
        frame.tick = xTaskGetTickCountFromISR();
        memcpy(frame.data, rxData, sizeof(frame.data));

        if ((can1RxQueue == NULL) ||
            (xQueueSendFromISR(can1RxQueue, &frame, &xHigherPriorityTaskWoken) != pdPASS))
        {
            can1RxOverflowCount++;
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1)
    {
        can1LastErrorCode = HAL_CAN_GetError(hcan);
    }
}
