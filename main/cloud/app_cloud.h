#pragma once
#ifndef __APP_SNTP_H__
#define __APP_SNTP_H__

#include "esp_sntp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _LOCATION {
    char ip[40];
    char country_code[16];
    char country_name[32];
    char province[32];
    char city[32];
    float latitude;
    float longitude;
    int   timezone;
} LOCATION;

typedef struct _WEATHER {
    short    year;
    uint8_t  month;
    uint8_t  day;
    int      temp_max;
    int      temp_min;
    int      day_code;
    char     day_text[16];
    char     day_wind_dir[16];
    char     day_wind_scale[16];
    int      night_code;
    char     night_text[16];
    char     night_wind_dir[16];
    char     night_wind_scale[16];
    int      pressure;
    int      humidity;
} WEATHER;

// "date":"2023/07/31",
// "high":32,
// "dayText":"雷阵雨",
// "dayCode":4,
// "dayWindDirection":"无持续风向",
// "dayWindScale":"微风",
// "low":27,
// "nightText":"雷阵雨",
// "nightCode":4,
// "nightWindDirection":"无持续风向",
// "nightWindScale":"微风"

// "fxDate":"2023-07-25",
// "sunrise":"05:52",
// "sunset":"19:10",
// "moonrise":"11:58",
// "moonset":"23:46",
// "moonPhase":"峨眉月",
// "moonPhaseIcon":"801",
// "tempMax":"34",
// "tempMin":"28",
// "iconDay":"100",
// "textDay":"晴",
// "iconNight":"150",
// "textNight":"晴",
// "wind360Day":"0",
// "windDirDay":"北风",
// "windScaleDay":"1-2",
// "windSpeedDay":"3",
// "wind360Night":"0",
// "windDirNight":"北风",
// "windScaleNight":"1-2",
// "windSpeedNight":"3",
// "humidity":"82",
// "precip":"0.0",
// "pressure":"995",
// "vis":"19",
// "cloud":"25",
// "uvIndex":"8"

void cloud_sync();

int sync_elapsed_time();

bool app_sntp_sync_time(sntp_sync_time_cb_t callback);

const LOCATION * app_get_locatioin();

#ifdef __cplusplus
}
#endif

#endif	// __APP_SNTP_H__
