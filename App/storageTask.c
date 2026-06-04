#include "storageTask.h"

QueueHandle_t g_StorageQueue = NULL;
QueueHandle_t g_StorageDataQueue = NULL;

static FATFS myFatFs;
static uint8_t fatfs_mounted = 0U;

const char *storageCounrt = "x0";
uint8_t isRecording = 0;    // 是否正在存储
int storage_count = 0;      // 当前存储数量

FRESULT FATFS_Mount0_Auto(void)
{
    FRESULT f_res;

    if (fatfs_mounted != 0U)
    {
        return FR_OK;
    }

    f_res = f_mount(&myFatFs, "0:", 1);

    if (f_res == FR_NO_FILESYSTEM)
    {
        printf("[storage] No filesystem, formatting...\r\n");

        static uint8_t aMountBuffer[4096];

        f_res = f_mkfs("0:", 0, 0, aMountBuffer, sizeof(aMountBuffer));
        if (f_res != FR_OK)
        {
            printf("[storage] Format failed: %d\r\n", f_res);
            fatfs_mounted = 0U;
            return f_res;
        }

        printf("[storage] Format success\r\n");

        f_mount(NULL, "0:", 1);

        f_res = f_mount(&myFatFs, "0:", 1);
        if (f_res != FR_OK)
        {
            printf("[storage] Mount failed after format: %d\r\n", f_res);
            fatfs_mounted = 0U;
            return f_res;
        }

        printf("[storage] Mount success\r\n");
        fatfs_mounted = 1U;
        return FR_OK;
    }
    else if (f_res != FR_OK)
    {
        printf("[storage] Mount error: %d\r\n", f_res);
        fatfs_mounted = 0U;
        return f_res;
    }

    printf("[storage] Mount success\r\n");
    fatfs_mounted = 1U;

    return FR_OK;
}

void Storage_Init(void)
{
    if (g_StorageQueue == NULL)
    {
        g_StorageQueue = xQueueCreate(STORAGE_CMD_QUEUE_LEN, sizeof(StorageCmd_t));
        configASSERT(g_StorageQueue != NULL);
    }

    if (g_StorageDataQueue == NULL)
    {
        g_StorageDataQueue = xQueueCreate(STORAGE_DATA_QUEUE_LEN, sizeof(sensor_data_t));
        configASSERT(g_StorageDataQueue != NULL);
    }
}

static void updateStorageCount(ScreenPage page, const char *obj, int count)
{
    char cmd[128];

    snprintf(cmd, sizeof(cmd),
             "%s.%s.val=%d",
             textStrings[page],
             obj,
             count);

    Screen_SendCmd(cmd);
}

void Storage_Task(void *argument)
{
    FIL file;                   // 文件对象
    FRESULT res;                // FatFS 返回值
    char fileName[64];          // 文件名
    uint32_t writeLen;

    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

    sensor_data_t recv_data;
    
    for (;;)
    {
        StorageCmd_t cmd;

        // ==========================================
        // 【非阻塞】检查有没有按键命令
        // ==========================================
        if (xQueueReceive(g_StorageQueue, &cmd, 0) == pdPASS)
        {
            if (cmd == CMD_START && isRecording == 0)
            {
                // 查看是否挂载成功
                if (FATFS_Mount0_Auto() == FR_OK)
                {
                    // 获取时间戳
                    taskENTER_CRITICAL();
                    HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
                    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);
                    sprintf(fileName, "water_test_20%02d_%02d_%02d_%02d_%02d_%02d.csv", RTC_DateStruct.Year,
                            RTC_DateStruct.Month,
                            RTC_DateStruct.Date,
                            RTC_TimeStruct.Hours,
                            RTC_TimeStruct.Minutes,
                            RTC_TimeStruct.Seconds);
                    taskEXIT_CRITICAL();

                    // 新建文件
                    res = f_open(&file, fileName, FA_CREATE_ALWAYS | FA_WRITE);

                    if (res == FR_OK)
                    {
                        const char *header = "ph,temperature,turbidity,conductivity\r\n";
                        f_write(&file, header, strlen(header), &writeLen);

                        isRecording = 1;
                        printf("start storage %s \r\n", fileName);
                        storage_count = 0;
                        updateStorageCount(STORAGE_PAGE, storageCounrt, storage_count);
                        osDelay(50);
                    }
                }
                else
                {
                    printf("[storage] disk mount faild \r\n");
                }
            }
            else if (cmd == CMD_STOP && isRecording == 1)
            {
                // 停止存储
                isRecording = 0;
                f_close(&file);
                printf("[storage] stop storage and file saved successfully \n");
            }
        }

        // ==========================================
        // 【关键】如果正在存储，一直写数据
        // ==========================================
        if (isRecording == 1 && xQueueReceive(g_StorageDataQueue, &recv_data, 0) == pdPASS)
        {
            char buf[64];
            sprintf(buf, "%.2f,%.2f,%.2f,%.2f\r\n", recv_data.ph,
                    recv_data.temperature,
                    recv_data.turbidity,
                    recv_data.conductivity);

            // 写入文件
            f_write(&file, buf, strlen(buf), &writeLen);
            
            ++storage_count;
            // 每隔5条刷新一次
            if (storage_count % 5 == 0) 
            {
                updateStorageCount(STORAGE_PAGE, storageCounrt, storage_count);
            }
            // 复位
            if (storage_count >= 50000)
                storage_count = 0;
            
            osDelay(1);
        }
        else
        {
            if (isRecording == 1)
            {
                osDelay(1); // 正在存储，只短暂让出 CPU
            }
            else
            {
                LED0_Troggle();
                osDelay(1000); // 没在存储，才慢速闪灯
            }
        }
    }
}
