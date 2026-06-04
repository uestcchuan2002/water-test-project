#include "displayTask.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "sdio.h"
#include "canRxTask.h"

const char *textStrings[] = {
    "rs485_para",
    "adc_para",
    "can_para",
    "ph_cal",
    "ec_cal",
    "storage"
};

volatile ScreenPage currentPage = RS485_PARA_PAGE;

static void Screen_SetTextFloat(ScreenPage page, const char *obj, float value, uint8_t decimals);
void update_And_printf_calibration_parameter(void);

void DisplayTask_SetPage(ScreenPage page)
{
    currentPage = page;
}

ScreenPage DisplayTask_GetPage(void)
{
    return currentPage;
}

void displayTask(void)
{
    DisplayTask_Run(NULL);
}

void DisplayTask_Run(void *argument)
{
    sensor_data_t adc_data = {0};
    CanSensorSample_t can_data;
    CanSensorSample_t can_latest[CAN_SENSOR_COUNT];

    /* 初始赋值为0，防止显示错误 */
    for (int i = 0; i < CAN_SENSOR_COUNT; i++) 
    {
        can_latest[i].value = 0.0f;
    }

    TickType_t lastRefreshTick = 0;

    (void)argument;

    configASSERT(sensorDataQueue != NULL);
    configASSERT(canSensorDataQueue != NULL);

    AT24CXX_Init();
    update_And_printf_calibration_parameter();
    
    osDelay(500);
    
    while (1)
    {
        /* 1. ADC队列采用短超时等待，避免 DisplayTask 永久卡死在ADC队列 */
        if (xQueueReceive(sensorDataQueue, &adc_data, pdMS_TO_TICKS(20)) == pdPASS) 
        {
            // adc_data 已经是最新 ADC 数据
        }

        /* 2. CAN 队列用非阻塞方式清空，只保留最新值 */
        while (xQueueReceive(canSensorDataQueue, &can_data, 0) == pdPASS) 
        {
            uint8_t index = (uint8_t)can_data.type - 1U;

            if (index < CAN_SENSOR_COUNT) 
            {
                can_latest[index] = can_data;
            }
        }

        /* 3. 屏幕定期更新 */
        if (xTaskGetTickCount() - lastRefreshTick >= pdMS_TO_TICKS(500)) 
        {
            lastRefreshTick = xTaskGetTickCount();

            switch (DisplayTask_GetPage())
            {
                case ADC_PARA_PAGE:
                {
                    Screen_SetTextFloat(ADC_PARA_PAGE, "ph", adc_data.ph, 2);
                    osDelay(50);
                    Screen_SetTextFloat(ADC_PARA_PAGE, "temp", adc_data.temperature, 2);
                    osDelay(50);
                    Screen_SetTextFloat(ADC_PARA_PAGE, "tur", adc_data.turbidity, 2);
                    osDelay(50);
                    Screen_SetTextFloat(ADC_PARA_PAGE, "cond", adc_data.conductivity, 2);
                }
                break;
                case CAN_PARA_PAGE:
                {
                    Screen_SetTextFloat(CAN_PARA_PAGE, "yulv", can_latest[0].value, 2);
                    osDelay(50);
                    Screen_SetTextFloat(CAN_PARA_PAGE, "rong", can_latest[1].value, 2);
                    osDelay(50);
                    Screen_SetTextFloat(CAN_PARA_PAGE, "tie", can_latest[2].value, 2);
                    osDelay(50);
                    Screen_SetTextFloat(CAN_PARA_PAGE, "tong", can_latest[3].value, 2);
                    osDelay(50);
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

void update_And_printf_calibration_parameter(void)
{
    AT24CXX_Read(0, (uint8_t *)&calibration_para, sizeof(calibration_para_t));
    // 打印所有校准参数
    printf("=============== Calibration Para ==========\r\n");
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
    printf("===========================================\r\n");
}	

