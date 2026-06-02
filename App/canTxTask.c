#include "canTxTask.h"

void CanTxTask_Run(void * argument)
{
    uint8_t canbuf[8];
    uint8_t res = 0;
    uint8_t cnt = 0;
    uint8_t tbuf[40];
    CAN1_Mode_Init(CAN_SJW_1TQ, CAN_BS2_6TQ, CAN_BS1_7TQ, 6, CAN_MODE_LOOPBACK);
    CAN_Config();

	while(1) 
	{
		for (int i = 0; i < 8; i++) 
        {
            canbuf[i] = cnt + i;
        }
        sprintf((char*)tbuf, "%d %d %d %d %d %d %d %d",canbuf[0], canbuf[1],canbuf[2], canbuf[3],canbuf[4], canbuf[5],canbuf[6], canbuf[7]);
        printf("tx date: %s\r\n", tbuf);
        res = CAN1_Send_Msg(canbuf, 8);

        if (res) 
        {
            lcd_show_string(30, 250, 300, 16, 32, (char*)"Tx Failed   ", RED);
        }
        else 
        {
            lcd_show_string(30, 250, 300, 16, 32, (char*)"Tx Success  ", RED);
        }

        cnt++;
		osDelay(2000);
	}
}

