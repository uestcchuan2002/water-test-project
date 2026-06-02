#include "can.h"

CAN_HandleTypeDef	CAN1_Handler;   	//	CAN1句柄
CAN_TxHeaderTypeDef	TxHeader;      		//	发送
CAN_RxHeaderTypeDef	RxHeader;      		//	接收

/*
	1. CAN初始化
	2. tsjw: 重新同步跳跃时间单元. 范围: CAN_SJW_1TQ ~ CAN_SJW_4TQ
	3. tbs2: 时间段2的时间单元.    范围: CAN_BS2_1TQ ~ CAN_BS2_8TQ
	4. tbs1: 时间段1的时间单元.    范围: CAN_BS1_1TQ ~ CAN_BS1_16TQ
	5. brp : 波特率分频器. 		   范围: 1 ~ 1024; tq = (brp) * tpclk1
	6. mode: CAN_MODE_NORMAL,普通模式; CAN_MODE_LOOPBACK,回环模式;
	7. example:
	波特率 = Fpclk1/((tbs1+tbs2+1)*brp); 其中tbs1和tbs2我们只用关注标识符上标志的序号，例如CAN_BS2_1TQ，我们就认为tbs2=1来计算即可。
	Fpclk1的时钟在初始化的时候设置为42M,如果设置CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,CAN_MODE_LOOPBACK);
	则波特率为:42M/((6+7+1)*6)=500Kbps
	返回值:0,初始化OK;
	其他,初始化失败;
*/
uint8_t CAN1_Mode_Init(uint32_t tsjw, uint32_t tbs2, uint32_t tbs1, uint32_t brp, uint32_t mode)
{
	CAN_InitTypeDef CAN_InitConf;
	
	CAN1_Handler.Instance = CAN1;
	CAN1_Handler.Init = CAN_InitConf;
	
	/* 预分频，APB1总线 */
	CAN1_Handler.Init.Prescaler = brp;
	/* 模式设置 */
	CAN1_Handler.Init.Mode = mode;
	/* 设置再同步的补偿最大值 */
	CAN1_Handler.Init.SyncJumpWidth = tsjw;
	/* tbs1大小 */
	CAN1_Handler.Init.TimeSeg1 = tbs1;		
	/* tbs2大小 */	
    CAN1_Handler.Init.TimeSeg2 = tbs2;				
	/* 非时间触发通信模式  */
    CAN1_Handler.Init.TimeTriggeredMode = DISABLE;	
	/* 软件自动离线管理  */
    CAN1_Handler.Init.AutoBusOff = DISABLE;			
	/* 睡眠模式通过软件唤醒 */
    CAN1_Handler.Init.AutoWakeUp = DISABLE;			
	/* 禁止报文自动传送 */
    CAN1_Handler.Init.AutoRetransmission=ENABLE;	
	/* 报文不锁定,新的覆盖旧的  */
    CAN1_Handler.Init.ReceiveFifoLocked=DISABLE;	
	/* 优先级由报文标识符决定  */
    CAN1_Handler.Init.TransmitFifoPriority=DISABLE;
	
	/* 初始化 */
	if(HAL_CAN_Init(&CAN1_Handler) != HAL_OK) 
	{
		return 1;
	}
		
    return 0;
}

/*
	1. CAN底层驱动，引脚配置，时钟配置，中断配置
	2. 此函数会被HAL_CAN_Init()调用
	3. hcan:CAN句柄
*/
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_CAN1_CLK_ENABLE();                // 使能CAN1时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();			    // 开启GPIOA时钟
	
    GPIO_Initure.Pin=GPIO_PIN_11|GPIO_PIN_12;   // PA11,12
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          // 推挽复用
    GPIO_Initure.Pull=GPIO_PULLUP;              // 上拉
    GPIO_Initure.Speed=GPIO_SPEED_FAST;         // 快速
    GPIO_Initure.Alternate=GPIO_AF9_CAN1;       // 复用为 CAN1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);         // 初始化
}

/*
	1. can的过滤器配置
	2. 开启can外设
	3. 激活can接收邮箱
	4. 配置发送参数
*/
void CAN_Config(void)
{
	CAN_FilterTypeDef  sFilterConfig;

	/*##-1- Configure the CAN Filter ###########################################*/
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
		/* Filter configuration Error */
		while(1)
		{
		}
	}

	/*##-2- Start the CAN peripheral ###########################################*/
	if (HAL_CAN_Start(&CAN1_Handler) != HAL_OK)
	{
		/* Start Error */
		while(1)
		{
		}
	}

	/*##-3- Activate CAN RX notification #######################################*/
	if (HAL_CAN_ActivateNotification(&CAN1_Handler, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		/* Notification Error */
		while(1)
		{
		}
	}

	/*##-4- Configure Transmission process #####################################*/
	TxHeader.StdId = 0x321;
	TxHeader.ExtId = 0x01;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = 2;
	TxHeader.TransmitGlobalTime = DISABLE;
}



/*
	1. can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)	
	2. len:数据长度(最大为8)				     
	3. msg:数据指针,最大为8个字节.
	4. 返回值:0,成功;
		 其他,失败;
*/
uint8_t CAN1_Send_Msg(uint8_t* msg, uint8_t len)
{	
    uint8_t i=0;
	uint32_t TxMailbox;
	uint8_t message[8];
    TxHeader.StdId = 0X12;        //标准标识符
    TxHeader.ExtId = 0x12;        //扩展标识符(29位)
    TxHeader.IDE = CAN_ID_STD;    //使用标准帧
    TxHeader.RTR = CAN_RTR_DATA;  //数据帧
    TxHeader.DLC = len;                
    for(i = 0; i < len; i++)
    {
		message[i] = msg[i];
	}
    if(HAL_CAN_AddTxMessage(&CAN1_Handler, &TxHeader, message, &TxMailbox) != HAL_OK)//发送
	{
		return 1;
	}
	while(HAL_CAN_GetTxMailboxesFreeLevel(&CAN1_Handler) != 3) {}
    return 0;
}

/*
	1. can口接收数据查询
	2. buf:数据缓存区;	 
	3. 返回值:0,无数据被收到;
		 其他,接收的数据长度;
*/
uint8_t CAN1_Receive_Msg(uint8_t *buf)
{
 	uint32_t i;
	uint8_t	RxData[8];

	if(HAL_CAN_GetRxFifoFillLevel(&CAN1_Handler, CAN_RX_FIFO0) != 1)
	{
		return 0xF1;
	}

	if(HAL_CAN_GetRxMessage(&CAN1_Handler, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		return 0xF2;
	}
	
    for(i = 0; i < RxHeader.DLC; i++) 
	{
		buf[i]=RxData[i];
	}
    
	return RxHeader.DLC;
}

