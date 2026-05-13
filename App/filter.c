#include "filter.h"




void Median5_Init(Median5_t *f)
{
    memset(f, 0, sizeof(Median5_t));
}

void MovingAvg_Init(MovingAvg_t *f)
{
    memset(f, 0, sizeof(MovingAvg_t));
}

/* 5点中值滤波：排序网络，比通用排序更适合MCU */
uint16_t Median5_Update(Median5_t *f, uint16_t in)
{
    f->buf[f->idx] = in;
    f->idx = (f->idx + 1) % MEDIAN_N;
    if (f->count < MEDIAN_N) f->count++;

    uint16_t x0 = f->buf[0];
    uint16_t x1 = f->buf[1];
    uint16_t x2 = f->buf[2];
    uint16_t x3 = f->buf[3];
    uint16_t x4 = f->buf[4];

    if (f->count < MEDIAN_N) {
        uint16_t tmp[MEDIAN_N] = {x0, x1, x2, x3, x4};
        for (uint8_t i = 1; i < f->count; i++) {
            uint16_t key = tmp[i];
            int8_t j = i - 1;
            while (j >= 0 && tmp[j] > key) {
                tmp[j + 1] = tmp[j];
                j--;
            }
            tmp[j + 1] = key;
        }
        return tmp[f->count / 2];
    }

    SWAP_U16(x0, x1); SWAP_U16(x3, x4);
    SWAP_U16(x0, x2); SWAP_U16(x1, x2);
    SWAP_U16(x3, x2); SWAP_U16(x4, x2);
    SWAP_U16(x1, x3); SWAP_U16(x2, x3);
    SWAP_U16(x1, x2);

    return x2;
}

/* 整型滑动平均 */
uint16_t MovingAvg_Update(MovingAvg_t *f, uint16_t in)
{
    if (f->count < AVG_N) {
        f->count++;
    } else {
        f->sum -= f->buf[f->idx];
    }

    f->buf[f->idx] = in;
    f->sum += in;
    f->idx = (f->idx + 1) % AVG_N;

    return (uint16_t)((f->sum + f->count / 2) / f->count);  // 四舍五入
}

/* 组合滤波：先中值，再平均 */
uint16_t Filter_Update(Median5_t *mf, MovingAvg_t *af, uint16_t raw)
{
    uint16_t mid = Median5_Update(mf, raw);
    return MovingAvg_Update(af, mid);
}


