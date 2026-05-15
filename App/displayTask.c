#include "displayTask.h"


void displayTask(void)
{
    sensor_data_t recv_data;
    while (1)
    {
        if (xQueueReceive(sensorDataQueue, &recv_data, portMAX_DELAY) == pdPASS)
        {
            printf("ph:%.3lf\r\n", recv_data.ph);
            printf("temperature:%.3lf\r\n", recv_data.temperature);
            printf("turbidity:%.3lf\r\n", recv_data.turbidity);
            printf("conductivity:%.3lf\r\n", recv_data.conductivity);
            printf("\r\n");
        }
    }
}


