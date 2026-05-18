#include "displayTask.h"

void displayTask(void)
{
    sensor_data_t recv_data;
    ui_water_data_t ui_data;

    lcd_init();
    ui_draw_static();

    while (1)
    {
        if (xQueueReceive(sensorDataQueue, &recv_data, portMAX_DELAY) == pdPASS)
        {
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
