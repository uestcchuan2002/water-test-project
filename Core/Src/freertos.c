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
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for taskADC */
osThreadId_t taskADCHandle;
const osThreadAttr_t taskADC_attributes = {
  .name = "taskADC",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for taskAdcDataProc */
osThreadId_t taskAdcDataProcHandle;
const osThreadAttr_t taskAdcDataProc_attributes = {
  .name = "taskAdcDataProc",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal7,
};
/* Definitions for taskDisplay */
osThreadId_t taskDisplayHandle;
const osThreadAttr_t taskDisplay_attributes = {
  .name = "taskDisplay",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal6,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void appTaskLED(void *argument);
void appTaskADC(void *argument);
void appTaskAdcDataProc(void *argument);
void appTaskDisplay(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
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

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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
  for(;;)
  {
    LED0_Troggle();
    osDelay(500);
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
    adcTask();
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
  adcDataPrcoTask();
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
  /* Infinite loop */
  displayTask();
  /* USER CODE END appTaskDisplay */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

