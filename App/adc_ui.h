#ifndef __ADC_UI_H
#define __ADC_UI_H


#include "main.h"
#include "lcd.h"
#include <stdio.h>

#define LCD_W   320
#define LCD_H   480

#define UI_BG           WHITE
#define UI_HEADER       BLUE
#define UI_CARD_BG      0xF7BE      /* ºÜÇ³µÄ»Ò°×É« */
#define UI_TEXT         BLACK
#define UI_TEXT_LIGHT   GRAY
#define UI_OK           GREEN
#define UI_WARN         YELLOW
#define UI_ALARM        RED

typedef struct
{
    float ph;
    float temp;
    float turbidity;
    float conductivity;
    uint8_t rs485_ok;
    uint8_t alarm;
} ui_water_data_t;

void ui_draw_static(void);
void ui_update_data(ui_water_data_t *data);

#endif 
