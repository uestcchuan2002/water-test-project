#ifndef __CAN_H
#define __CAN_H

#include "main.h"

uint8_t CAN1_Mode_Init(uint32_t tsjw, uint32_t tbs2, uint32_t tbs1, uint32_t brp, uint32_t mode);
void CAN_Config(void);
uint8_t CAN1_Send_Msg(uint8_t* msg, uint8_t len);
uint8_t CAN1_Receive_Msg(uint8_t *buf);

#endif
