#ifndef __DISPLAYTASK_H
#define __DISPLAYTASK_H

#include "main.h"
#include "stdio.h"
#include "queue.h"
#include "adcDataProcTask.h"
#include "adcTask.h"
#include "lcd.h"
#include "adc_ui.h"
#include "24cxx.h"
#include "led.h"
#include "screenTxTask.h"

void displayTask(void);

typedef enum
{
    RS485_PARA_PAGE,
    ADC_PARA_PAGE,
    PH_CAL_PAGE,
    EC_CAL_PAGE,
    STORAGE_PAGE
} ScreenPage;

extern volatile ScreenPage currentPage;

void DisplayTask_Run(void *argument);
void DisplayTask_SetPage(ScreenPage page);
ScreenPage DisplayTask_GetPage(void);

#endif
