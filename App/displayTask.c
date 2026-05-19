#include "displayTask.h"

const char *textStrings[] = {
    "rs485_para",
    "adc_para",
    "ph_cal"
};

typedef enum {
    RS485_PARA_PAGE,
    ADC_PARA_PAGE,
    PH_CAL
} ScreenPage;

static void Screen_SetText(ScreenPage page, const char *obj, const char *text);
static void Screen_SetText(ScreenPage page, const char *obj, const char *text);
static void Screen_SetTextFloat(ScreenPage page, const char *obj, float value, uint8_t decimals);


const uint8_t TEXT_Buffer[] = {"stm32 study of at24c02.."};
#define SIZE sizeof(TEXT_Buffer)
uint8_t datatemp[SIZE];
char cmd[64];


void displayTask(void)
{
    sensor_data_t recv_data;
    ui_water_data_t ui_data;
    float ph_test = 1.00;

    lcd_init();
    ui_draw_static();

    lcd_init();
    AT24CXX_Init();
    
    while (AT24CXX_Check()) // 检测不到24c02
    {
        LED0_Troggle();
    }
    // AT24CXX_Write(0,(uint8_t*)TEXT_Buffer,SIZE);

    AT24CXX_Read(0, datatemp, SIZE);
    printf("read: %s\r\n", datatemp);

    while (1)
    {
        if (xQueueReceive(sensorDataQueue, &recv_data, portMAX_DELAY) == pdPASS)
        {
            ph_test += 0.01f;
            if (ph_test > 14.0f) {
                ph_test = 0.0f;
            }
            // Screen_SetTextFloat(RS485_PARA_PAGE, "ph", ph_test, 2);

            ui_data.ph = recv_data.ph;
            ui_data.temp = recv_data.temperature;
            ui_data.turbidity = recv_data.turbidity;
            ui_data.conductivity = recv_data.conductivity;
            ui_data.rs485_ok = 1;
            ui_data.alarm = 0;

            ui_update_data(&ui_data);
        }
    }
}

static void Screen_SetText(ScreenPage page, const char *obj, const char *text)
{
    char cmd[128];

    snprintf(cmd, sizeof(cmd), "%s.%s.txt=\"%s\"", textStrings[page],  obj, text);
    Screen_SendCmd(cmd);
}

static void Screen_SetTextInt(ScreenPage page, const char *obj, int value)
{
    char cmd[128];

    snprintf(cmd, sizeof(cmd), "%s.%s.txt=\"%d\"", textStrings[page], obj, value);
    Screen_SendCmd(cmd);
}

static void Screen_SetTextFloat(ScreenPage page, const char *obj, float value, uint8_t decimals)
{
    char cmd[128];

    snprintf(cmd, sizeof(cmd),
             "%s.%s.txt=\"%.*f\"",
             textStrings[page],
             obj,
             decimals,
             value);

    Screen_SendCmd(cmd);
}


