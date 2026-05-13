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
    if (f->count > 0) {
        uint16_t base = f->last;
        uint16_t diff = (in > base) ? (in - base) : (base - in);

        if (diff > FILTER_DIFF_MAX) {
            f->abnormal_count++;

            if (f->abnormal_count < FILTER_ABN_MAX) {
                return f->last;   // 偶发异常，丢弃
            }

            f->abnormal_count = 0; // 连续异常，认为可能是真实变化
        } else {
            f->abnormal_count = 0;
        }
    }

    f->buf[f->idx] = in;
    f->idx = (f->idx + 1) % MEDIAN_N;
    if (f->count < MEDIAN_N) {
        f->count++;
    }

    uint16_t tmp[MEDIAN_N];
    for (uint8_t i = 0; i < f->count; i++) {
        tmp[i] = f->buf[i];
    }

    for (uint8_t i = 1; i < f->count; i++) {
        uint16_t key = tmp[i];
        int8_t j = i - 1;

        while (j >= 0 && tmp[j] > key) {
            tmp[j + 1] = tmp[j];
            j--;
        }

        tmp[j + 1] = key;
    }

    f->last = tmp[f->count / 2];
    return f->last;
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


