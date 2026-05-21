#include "displayTask.h"
#include "fatfs.h"
#include "sdio.h"
const char *textStrings[] = {
    "rs485_para",
    "adc_para",
    "ph_cal",
    "ec_cal"};


static void Screen_SetTextFloat(ScreenPage page, const char *obj, float value, uint8_t decimals);

volatile ScreenPage currentPage = RS485_PARA_PAGE;

void SDCardInfo(void);
void FatFsTest(void);
void update_And_printf_calibration_parameter(void);

void displayTask(void)
{
    sensor_data_t recv_data;

	FatFsTest();
	SDCardInfo();
	
    lcd_init();
    ui_draw_static();

    AT24CXX_Init();
    update_And_printf_calibration_parameter();

    while (AT24CXX_Check()) {
        // 检测不到24c02
        LED0_Troggle();
    }

    // 从EEPROM中读取校准参数
    osDelay(500);
    
    while (1)
    {
        if (xQueueReceive(sensorDataQueue, &recv_data, portMAX_DELAY) == pdPASS)
        {            
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

void FatFsTest(void)
{
    static FATFS myFatFs;
    static FIL myFile;
    static FRESULT f_res;
    static UINT num;
    static uint8_t aReadData[1024] = {0};
    static uint8_t aWriteBuf[] = "hello world this is fatfs test\r\n";

    HAL_Delay(1000);

    printf("\r\n\r\n");
    printf("1. Mount FatFs test ****** \r\n");

    f_res = f_mount(&myFatFs, "0:", 1);
    if (f_res == FR_NO_FILESYSTEM)
    {
        printf("No filesystem, formatting...\r\n");
        static uint8_t aMountBuffer[4096];
        f_res = f_mkfs("0:", 0, 0, aMountBuffer, sizeof(aMountBuffer));
        if (f_res == FR_OK)
        {
            printf("Format success\r\n");
            f_mount(NULL, "0:", 1);
            f_res = f_mount(&myFatFs, "0:", 1);
            if (f_res == FR_OK)
                printf("Mount success\r\n");
            else
                return;
        }
        else
        {
            printf("Format failed\r\n");
            return;
        }
    }
    else if (f_res != FR_OK)
    {
        printf("Mount error: %d\r\n", f_res);
        return;
    }
    else
    {
        printf("Mount success\r\n");
    }

    printf("\r\n");
    printf("2. Write test ****** \r\n");
    f_res = f_open(&myFile, "0:/text.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (f_res == FR_OK)
    {
        f_res = f_write(&myFile, aWriteBuf, sizeof(aWriteBuf) - 1, &num);
        if (f_res == FR_OK)
        {
            printf("Write success\r\n");
            printf("Written bytes: %u\r\n", num);
        }
        else
        {
            printf("Write failed: %d\r\n", f_res);
        }
        f_close(&myFile);
    }
    else
    {
        printf("Open file failed: %d\r\n", f_res);
    }

    printf("3. Read test ****** \r\n");
    memset(aReadData, 0, sizeof(aReadData));
    f_res = f_open(&myFile, "0:/text.txt", FA_OPEN_EXISTING | FA_READ);
    if (f_res == FR_OK)
    {
        f_res = f_read(&myFile, aReadData, sizeof(aReadData) - 1, &num);
        if (f_res == FR_OK)
        {
            aReadData[num] = '\0';
            printf("Read success\r\n");
            printf("Read bytes: %u\r\n", num);
            printf("Read data: %s\r\n", aReadData);
        }
        else
        {
            printf("Read failed: %d\r\n", f_res);
        }
        f_close(&myFile);
    }
    else
    {
        printf("Open file failed: %d\r\n", f_res);
    }

    f_mount(NULL, "0:", 1);
}

void SDCardInfo(void)
{
    HAL_SD_CardInfoTypeDef pCardInfo = {0};
    HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd);

    if (state == HAL_SD_CARD_TRANSFER)
    {
        if (HAL_SD_GetCardInfo(&hsd, &pCardInfo) == HAL_OK)
        {
            printf("\r\n*** Get SD card info ***\r\n");
            printf("CardType: %lu\r\n", (unsigned long)pCardInfo.CardType);
            printf("CardVersion: %lu\r\n", (unsigned long)pCardInfo.CardVersion);
            printf("BlockNbr: %lu\r\n", (unsigned long)pCardInfo.BlockNbr);
            printf("BlockSize: %lu\r\n", (unsigned long)pCardInfo.BlockSize);
            printf("Capacity: %lu MB\r\n",
                   (uint32_t)(((uint64_t)pCardInfo.BlockSize * pCardInfo.BlockNbr) >> 20));
        }
        else
        {
            printf("HAL_SD_GetCardInfo failed\r\n");
        }
    }
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

