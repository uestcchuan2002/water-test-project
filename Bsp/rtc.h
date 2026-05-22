#ifndef __RTC_H
#define __RTC_H

#include "main.h"
#include "stdio.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//RTC驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2017/4/11 
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

extern RTC_HandleTypeDef RTC_Handler;  //RTC句柄
    
uint8_t RTC_Init(void);              		//RTC初始化
HAL_StatusTypeDef RTC_Set_Time(uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm);      //RTC时间设置
HAL_StatusTypeDef RTC_Set_Date(uint8_t year,uint8_t month,uint8_t date,uint8_t week);	//RTC日期设置
void RTC_Set_AlarmA(uint8_t week,uint8_t hour,uint8_t min,uint8_t sec); //设置闹钟时间(按星期闹铃,24小时制)
void RTC_Set_WakeUp(uint32_t wksel,uint16_t cnt);             //周期性唤醒定时器设置
#endif
