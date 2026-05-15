#ifndef __FILTER_H
#define __FILTER_H

#include <stdint.h>
#include <string.h>
#include "adcTask.h"


#define AVG_WIN 4

typedef struct
{
    uint16_t buf[AVG_WIN];
    uint8_t index;
    uint8_t count;
} moving_avg_t;

#endif
