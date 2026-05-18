#include "displayTask.h"
#include "24cxx.h"
#include "led.h"

const uint8_t TEXT_Buffer[]={"stm32 study of at24c02.."};
#define SIZE sizeof(TEXT_Buffer)
uint8_t datatemp[SIZE];	 

void displayTask(void)
{
    sensor_data_t recv_data;
    ui_water_data_t ui_data;

    lcd_init();
    ui_draw_static();

    lcd_init();
  AT24CXX_Init();	
  lcd_show_string(30,50,200,16,16,"Apollo STM32F4/F7", BLUE);
  while(AT24CXX_Check())//检测不到24c02
  {
    LED0_Troggle();
  }
  // AT24CXX_Write(0,(uint8_t*)TEXT_Buffer,SIZE);
 
  AT24CXX_Read(0,datatemp,SIZE);
  printf("read: %s\r\n", datatemp);

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
