#pragma once
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define msleep(n)           vTaskDelay(pdMS_TO_TICKS(n))


typedef struct _HUMITURE {
    float   temperature;
    float   humidity;
    time_t  meas_time;
} HUMITURE;

typedef struct _BATTERY {
    int     voltage;
    float   percent;
    time_t  meas_time;
} BATTERY;

typedef struct _SENSOR_METRIC {
    HUMITURE humiture;
    BATTERY  battery;
} SENSOR_METRIC;

void begin_sens_measure();

void end_sens_measure();

_Bool measue_humiture(HUMITURE * humiture);

void measue_battery(BATTERY * battery);


#ifdef __cplusplus
}
#endif
