#include "canRxTask.h"

void CanRxTask_Run(void * argument)
{
    uint8_t rev_count = 0;
    uint8_t canRxBuf[8];
    uint8_t tbuf[40];
	while(1) 
	{
        rev_count = CAN1_Receive_Msg(canRxBuf);
        if (rev_count == 8) {
            sprintf((char*)tbuf, "%d %d %d %d %d %d %d %d",canRxBuf[0], canRxBuf[1],
                                                            canRxBuf[2], canRxBuf[3],
                                                            canRxBuf[4], canRxBuf[5],
                                                            canRxBuf[6], canRxBuf[7]);
            printf("rx date: %s\r\n", tbuf);
        }
		osDelay(2000);
	}
}



