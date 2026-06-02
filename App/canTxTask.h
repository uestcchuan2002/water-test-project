#ifndef __CANTXTASK_H
#define __CANTXTASK_H

#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "can.h"
#include "lcd.h"

void CanTxTask_Run(void *argument);

#endif

