#ifndef __FILTER_H
#define __FILTER_H

#include <stdint.h>
#include <string.h>

#define MEDIAN_N   5
#define AVG_N      10

#define SWAP_U16(a, b) do { \
    uint16_t t = (a);        \
    (a) = (b);               \
    (b) = t;                 \
} while (0)


typedef struct {
    uint16_t buf[MEDIAN_N];
    uint8_t  idx;
    uint8_t  count;
} Median5_t;

typedef struct {
    uint16_t buf[AVG_N];
    uint8_t  idx;
    uint8_t  count;
    uint32_t sum;
} MovingAvg_t;

void Median5_Init(Median5_t *f);
void MovingAvg_Init(MovingAvg_t *f);
uint16_t Median5_Update(Median5_t *f, uint16_t in);
uint16_t MovingAvg_Update(MovingAvg_t *f, uint16_t in);
uint16_t Filter_Update(Median5_t *mf, MovingAvg_t *af, uint16_t raw);

#endif
