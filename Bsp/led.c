#include "led.h"

void LED0_Init(void) 
{
	HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin,  GPIO_PIN_SET);
}

void LED0_Troggle(void) 
{
	HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
}

void LED1_Init(void) 
{
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,  GPIO_PIN_SET);
}

void LED1_Troggle(void) 
{
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}


