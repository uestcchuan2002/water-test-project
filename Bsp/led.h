#ifndef __LED_H
#define __LED_H
 
#include "main.h"

#define LED0(x) x == 0 ?  \
													HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin,  GPIO_PIN_SET) \
													: HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin,  GPIO_PIN_RESET);

void LED0_Init(void);
void LED0_Troggle(void);
void LED1_Init(void);
void LED1_Troggle(void);

#endif 

