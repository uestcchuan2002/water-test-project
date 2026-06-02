#include "key.h"


void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           
    __HAL_RCC_GPIOE_CLK_ENABLE();           

    
    GPIO_Initure.Pin=GPIO_PIN_0;           
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      
    GPIO_Initure.Pull=GPIO_PULLDOWN;        
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;    
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    
	GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4; //PE2,3,4
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      
    GPIO_Initure.Pull=GPIO_PULLUP;          
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
    
}

uint8_t KEY_Scan(uint8_t mode)
{
    static uint8_t key_up=1;     
    if(mode==1)key_up=1;    
    printf("0\r\n");
    printf("after printf\r\n");
    if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
    {
		printf("1\r\n");
       
        key_up=0;
        if(KEY0==0)       return KEY0_PRES;
        else if(KEY1==0)  return KEY1_PRES;
        else if(KEY2==0)  return KEY2_PRES;
        else if(WK_UP==1) return WKUP_PRES;          
    }
    else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)
    {
        key_up=1;
        printf("2\r\n");
    }
    printf("3\r\n");     
    return 0;   
}
