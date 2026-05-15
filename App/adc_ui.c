#include "adc_ui.h"

static void ui_draw_card(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         char *name, char *value, uint16_t value_color)
{
    lcd_fill(x, y, x + w - 1, y + h - 1, UI_CARD_BG);
    lcd_draw_rectangle(x, y, x + w - 1, y + h - 1, LGRAY);

    lcd_show_string(x + 8, y + 8, 80, 24, 16, name, UI_TEXT_LIGHT);
    lcd_show_string(x + 8, y + 30, w - 16, 24, 24, value, value_color);
}

void ui_draw_static(void)
{
    lcd_clear(UI_BG);

    /* 顶部标题栏 */
    lcd_fill(0, 0, LCD_W - 1, 55, UI_CARD_BG);
    lcd_show_string(38, 16, 260, 24, 32, "Water Monitor", BLUE);

    /* 页面标题 */
    lcd_show_string(20, 70, 260, 24, 16, "Industrial Water Quality", UI_TEXT);

    /* 参数卡片背景 */
    ui_draw_card(16, 110, 288, 70, "pH", "--", UI_TEXT);
    ui_draw_card(16, 190, 288, 70, "Temp", "--", UI_TEXT);
    ui_draw_card(16, 270, 288, 70, "Turbidity", "--", UI_TEXT);
    ui_draw_card(16, 350, 288, 70, "Conductivity", "--", UI_TEXT);

    /* 底部状态栏 */
    lcd_fill(0, LCD_H - 44, LCD_W - 1, LCD_H - 1, LGRAY);
    lcd_show_string(16, LCD_H - 30, 140, 24, 24, "Status:", UI_TEXT);
}

void ui_update_data(ui_water_data_t *data)
{
    char buf[32];

    sprintf(buf, "%.2f", data->ph);
    ui_draw_card(16, 110, 288, 70, "pH", buf, data->alarm ? UI_ALARM : BLUE);

    sprintf(buf, "%.1f C", data->temp);
    ui_draw_card(16, 190, 288, 70, "Temp", buf, UI_TEXT);

    sprintf(buf, "%.0f NTU", data->turbidity);
    ui_draw_card(16, 270, 288, 70, "Turbidity", buf, UI_TEXT);

    sprintf(buf, "%.0f uS/cm", data->conductivity);
    ui_draw_card(16, 350, 288, 70, "Conductivity", buf, UI_TEXT);

    /* 刷新底部状态栏 */
    lcd_fill(0, LCD_H - 44, LCD_W - 1, LCD_H - 1, LGRAY);

    if (data->alarm)
    {
        lcd_show_string(16, LCD_H - 30, 150, 24, 24, "Status: Alarm", RED);
    }
    else
    {
        lcd_show_string(16, LCD_H - 30, 160, 24, 24, "Status: Normal", GREEN);
    }

    if (data->rs485_ok)
    {
        lcd_show_string(210, LCD_H - 30, 100, 24, 24, "RS485 OK", BLUE);
    }
    else
    {
        lcd_show_string(210, LCD_H - 30, 100, 24, 24, "RS485 ERR", RED);
    }
}
