#include "displayTask.h"

const char *textStrings[] = {
    "rs485_para",
    "adc_para",
    "ph_cal",
    "ec_cal"};


static void Screen_SetTextFloat(ScreenPage page, const char *obj, float value, uint8_t decimals);

volatile ScreenPage currentPage = RS485_PARA_PAGE;

void displayTask(void)
{
    sensor_data_t recv_data;
    // ui_water_data_t ui_data;
    float ph_test = 1.00;

    lcd_init();
    ui_draw_static();

    lcd_init();
    AT24CXX_Init();

    while (AT24CXX_Check()) {
        // 检测不到24c02
        LED0_Troggle();
    }

    // 从EEPROM中读取校准参数
    osDelay(500);
    AT24CXX_Read(0, (uint8_t *)&calibration_para, sizeof(calibration_para_t));
    // 打印所有校准参数
    printf("===== Calibration Para =====\r\n");
    printf("PH_CAL_TEMP_C   : %.2f\r\n", calibration_para.PH_CAL_TEMP_C);
    printf("PH_SLOPE_25C    : %.2f\r\n", calibration_para.PH_SLOPE_25C);
    printf("PH_OFFSET       : %.2f\r\n", calibration_para.PH_OFFSET);
    printf("TURB_A          : %.2f\r\n", calibration_para.TURB_A);
    printf("TURB_B          : %.2f\r\n", calibration_para.TURB_B);
    printf("TURB_C          : %.2f\r\n", calibration_para.TURB_C);
    printf("EC_ALPHA        : %.2f\r\n", calibration_para.EC_ALPHA);
    printf("EC_CAL_GAIN     : %.2f\r\n", calibration_para.EC_CAL_GAIN);
    printf("EC_CAL_OFFSET   : %.2f\r\n", calibration_para.EC_CAL_OFFSET);
    printf("EC_K_CELL       : %.2f\r\n", calibration_para.EC_K_CELL);
    printf("======================================\r\n");

    while (1)
    {
        if (xQueueReceive(sensorDataQueue, &recv_data, portMAX_DELAY) == pdPASS)
        {
            ph_test += 0.01f;
            if (ph_test > 14.0f)
            {
                ph_test = 0.0f;
            }
            
            switch (currentPage)
            {
                case RS485_PARA_PAGE:
                {
                    
                }
                break;
                case ADC_PARA_PAGE:
                {
                    Screen_SetTextFloat(ADC_PARA_PAGE, "ph", recv_data.ph, 2);
                    osDelay(100);
                    Screen_SetTextFloat(ADC_PARA_PAGE, "temp", recv_data.temperature, 2);
                    osDelay(100);
                    Screen_SetTextFloat(ADC_PARA_PAGE, "tur", recv_data.turbidity, 2);
                    osDelay(100);
                    Screen_SetTextFloat(ADC_PARA_PAGE, "cond", recv_data.conductivity, 2);
                }   
                break;
            }            
        }
    }
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
