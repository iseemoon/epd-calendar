#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void create_status_bar(lv_obj_t * parent);

int get_status_bar_height();

void refresh_status_bar();

#ifdef __cplusplus
}
#endif