#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_obj_t *parent;     // 父对象
    lv_obj_t *calendar;   // 日历对象
    // 其他必要的变量和数据结构
} custom_calendar_t;

custom_calendar_t *create_custom_calendar(lv_obj_t *parent);

void update_calender_date(int32_t year, uint8_t month, uint8_t day);

void lv_create_calender(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, size_t w, size_t h);

#ifdef __cplusplus
}
#endif