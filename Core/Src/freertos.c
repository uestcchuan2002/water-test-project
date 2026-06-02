/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "stdio.h"
#include "adcTask.h"
#include "adcDataProcTask.h"
#include "displayTask.h"
#include "screenTxTask.h"
#include "screenRxTask.h"
#include "storageTask.h"
#include "rtc.h"
#include "canTxTask.h"
#include "canRxTask.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for taskLED */
osThreadId_t taskLEDHandle;
const osThreadAttr_t taskLED_attributes = {
    .name = "taskLED",
    .stack_size = 128 * 24,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for taskADC */
osThreadId_t taskADCHandle;
const osThreadAttr_t taskADC_attributes = {
    .name = "taskADC",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityHigh,
};
/* Definitions for taskAdcDataProc */
osThreadId_t taskAdcDataProcHandle;
const osThreadAttr_t taskAdcDataProc_attributes = {
    .name = "taskAdcDataProc",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal7,
};
/* Definitions for taskDisplay */
osThreadId_t taskDisplayHandle;
const osThreadAttr_t taskDisplay_attributes = {
    .name = "taskDisplay",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal6,
};
/* Definitions for taskScreenTx */
osThreadId_t taskScreenTxHandle;
const osThreadAttr_t taskScreenTx_attributes = {
    .name = "taskScreenTx",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal1,
};
/* Definitions for taskScreenRx */
osThreadId_t taskScreenRxHandle;
const osThreadAttr_t taskScreenRx_attributes = {
    .name = "taskScreenRx",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal2,
};
/* Definitions for taskStorage */
osThreadId_t taskStorageHandle;
const osThreadAttr_t taskStorage_attributes = {
    .name = "taskStorage",
    .stack_size = 512 * 8,
    .priority = (osPriority_t)osPriorityAboveNormal2,
};
/* Definitions for taskCanTx */
osThreadId_t taskCanTxHandle;
const osThreadAttr_t taskCanTx_attributes = {
    .name = "taskCanTx",
    .stack_size = 512 * 8,
    .priority = (osPriority_t)osPriorityAboveNormal3,
};
/* Definitions for taskCanRx */
osThreadId_t taskCanRxHandle;
const osThreadAttr_t taskCanRx_attributes = {
    .name = "taskCanRx",
    .stack_size = 512 * 8,
    .priority = (osPriority_t)osPriorityAboveNormal3,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void appTaskLED(void *argument);
void appTaskADC(void *argument);
void appTaskAdcDataProc(void *argument);
void appTaskDisplay(void *argument);
void appTaskScreenTx(void *argument);
void appTaskScreenRx(void *argument);
void appTaskStorage(void *argument);
void appTaskCanTx(void *argument);
void appTaskCanRx(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */

    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    
    AdcTask_Init();
    AdcDataProcTask_Init();
    ScreenTx_Init();
    ScreenRx_Init();
    Storage_Init();

    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* creation of taskLED */
    taskLEDHandle = osThreadNew(appTaskLED, NULL, &taskLED_attributes);

    /* creation of taskADC */
    taskADCHandle = osThreadNew(appTaskADC, NULL, &taskADC_attributes);

    /* creation of taskAdcDataProc */
    taskAdcDataProcHandle = osThreadNew(appTaskAdcDataProc, NULL, &taskAdcDataProc_attributes);

    /* creation of taskDisplay */
    taskDisplayHandle = osThreadNew(appTaskDisplay, NULL, &taskDisplay_attributes);

    /* creation of taskScreenTx */
    taskScreenTxHandle = osThreadNew(appTaskScreenTx, NULL, &taskScreenTx_attributes);

    /* creation of taskScreenRx */
    taskScreenRxHandle = osThreadNew(appTaskScreenRx, NULL, &taskScreenRx_attributes);

    /* creation of taskStorage */
    taskStorageHandle = osThreadNew(appTaskStorage, NULL, &taskStorage_attributes);
	
	/* creation of taskCanTx */
    taskCanTxHandle = osThreadNew(appTaskCanTx, NULL, &taskCanTx_attributes);
	
    /* creation of taskCanRx */
    taskCanRxHandle = osThreadNew(appTaskCanRx, NULL, &taskCanRx_attributes);
    
    /* USER CODE BEGIN RTOS_THREADS */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_appTaskLED */
/**
 * @brief  Function implementing the taskLED thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskLED */
void appTaskLED(void *argument)
{
    /* USER CODE BEGIN appTaskLED */
    /* Infinite loop */
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    u8 tbuf[40];

    RTC_Init();
    lcd_init();

    for (;;)
    {
        HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
        sprintf((char *)tbuf, "Time:%02d:%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
        lcd_show_string(30, 10, 210, 16, 32, (char *)tbuf, BLUE);
        HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);
        sprintf((char *)tbuf, "Date:20%02d-%02d-%02d", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date);
        lcd_show_string(30, 50, 210, 16, 32, (char *)tbuf, BLUE);
        sprintf((char *)tbuf, "Week:%d", RTC_DateStruct.WeekDay);
        lcd_show_string(30, 90, 210, 16, 32, (char *)tbuf, BLUE);
        sprintf((char *)tbuf, "Page:%s", textStrings[currentPage]);
        lcd_show_string(30, 130, 300, 16, 32, (char *)tbuf, BLUE);
        sprintf((char *)tbuf, "Storage:%s", isRecording ? "ON " : "OFF");
        lcd_show_string(30, 170, 300, 16, 32, (char *)tbuf, BLUE);
        sprintf((char *)tbuf, "Sto_Num:%d", isRecording ? storage_count : 0);
        lcd_show_string(30, 210, 300, 16, 32, (char *)tbuf, BLUE);
        osDelay(1000);
    }
    /* USER CODE END appTaskLED */
}

/* USER CODE BEGIN Header_appTaskADC */
/**
 * @brief Function implementing the taskADC thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskADC */
void appTaskADC(void *argument)
{
    /* USER CODE BEGIN appTaskADC */
    AdcTask_Run(argument);
    /* USER CODE END appTaskADC */
}

/* USER CODE BEGIN Header_appTaskAdcDataProc */
/**
 * @brief Function implementing the taskAdcDataProc thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskAdcDataProc */
void appTaskAdcDataProc(void *argument)
{
    /* USER CODE BEGIN appTaskAdcDataProc */
    AdcDataProcTask_Run(argument);
    /* USER CODE END appTaskAdcDataProc */
}

/* USER CODE BEGIN Header_appTaskDisplay */
/**
 * @brief Function implementing the taskDisplay thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskDisplay */
void appTaskDisplay(void *argument)
{
    /* USER CODE BEGIN appTaskDisplay */
    DisplayTask_Run(argument);
    /* USER CODE END appTaskDisplay */
}

/* USER CODE BEGIN Header_appTaskScreenTx */
/**
 * @brief Function implementing the taskScreenTx thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskScreenTx */
void appTaskScreenTx(void *argument)
{
    /* USER CODE BEGIN appTaskScreenTx */
    ScreenTx_Task(argument);
    /* USER CODE END appTaskScreenTx */
}

/* USER CODE BEGIN Header_appTaskScreenRx */
/**
 * @brief Function implementing the taskScreenRx thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskScreenRx */
void appTaskScreenRx(void *argument)
{
    /* USER CODE BEGIN appTaskScreenRx */
    ScreenRx_Task(argument);
    /* USER CODE END appTaskScreenRx */
}

/* USER CODE BEGIN Header_appTaskStorage */
/**
 * @brief Function implementing the TaskStorage thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskStorage */
void appTaskStorage(void *argument)
{
    /* USER CODE BEGIN appTaskScreenRx */
    Storage_Task(argument);
    /* USER CODE END appTaskScreenRx */
}

/* USER CODE BEGIN Header_appTaskCanTx */
/**
 * @brief Function implementing the TaskCanTx thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskCanTx */
void appTaskCanTx(void *argument)
{
	CanTxTask_Run(argument);
}

/* USER CODE BEGIN Header_appTaskCanRx */
/**
 * @brief Function implementing the TaskCanRx thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_appTaskCanRx */
void appTaskCanRx(void *argument)
{
	CanRxTask_Run(argument);
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
