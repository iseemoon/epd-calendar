/**
 * @file lvgl_calendar.c
 *
 */
/*********************
 *      INCLUDES
 *********************/
#include "status_bar.h"
// #include "lvgl.h"
#include <stdlib.h>
#include <stdio.h>
#include "../common/common.h"
#include "../cloud/app_cloud.h"
#include "../misc/app_bat.h"


/*********************
 *      DEFINES
 *********************/
#define STATUS_BAR_HEIGHT   26
#define STATUS_BAR_BG_CLR   lv_color_make(0xFF, 0, 0)
#define STATUS_BAR_TXT_CLR  lv_color_white()

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
extern lv_font_t chinese_hei_16;
extern const lv_font_t lv_font_montserrat_16;
static lv_obj_t * lbl_location = NULL;
static lv_obj_t * lbl_update_tm = NULL;
// static lv_obj_t * lbl_bat_lvl = NULL;
static lv_obj_t * lbl_battery = NULL;
static lv_obj_t * lbl_wifi = NULL;
static lv_obj_t * lbl_bluetooth = NULL;


/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int get_status_bar_height()
{
    return STATUS_BAR_HEIGHT;
}

void create_status_bar(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, 0, 0);
    lv_obj_set_size(panel, lv_pct(100), STATUS_BAR_HEIGHT);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 3, LV_PART_MAIN);
    // lv_obj_set_style_pad_column(panel, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, STATUS_BAR_BG_CLR, 0);
    lv_obj_set_style_text_color(panel, STATUS_BAR_TXT_CLR, 0);
    lv_obj_set_style_text_font(panel, &chinese_hei_16, 0);

    // lv_obj_t * label = lv_label_create(panel);
    // lv_obj_set_style_pad_left(label, 10, 0);
    // lv_label_set_text_static(label, FW_VER);

    lv_obj_t * label = lv_label_create(panel);
    lv_obj_set_style_pad_left(label, 10, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_label_set_text_static(label, LV_SYMBOL_GPS);
    lbl_location = lv_label_create(panel);

    label = lv_label_create(panel);
    lv_obj_set_style_pad_left(label, 16, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_label_set_text_static(label, LV_SYMBOL_REFRESH);
    lbl_update_tm = lv_label_create(panel);
    lv_obj_set_flex_grow(lbl_update_tm, 1);


    lbl_bluetooth = lv_label_create(panel);
    lv_obj_set_style_text_font(lbl_bluetooth, &lv_font_montserrat_16, 0);
    lbl_wifi = lv_label_create(panel);
    lv_obj_set_style_text_font(lbl_wifi, &lv_font_montserrat_16, 0);
    lbl_battery = lv_label_create(panel);
    lv_obj_set_style_text_font(lbl_battery, &lv_font_montserrat_16, 0);
    // lbl_bat_lvl = lv_label_create(panel);
    lv_obj_set_style_pad_right(lbl_battery, 20, 0);
}

void refresh_status_bar()
{
    float bat_per = get_battery_percent(get_battery_vol());
    lv_label_set_text_static(lbl_wifi,      LV_SYMBOL_WIFI);
    // lv_label_set_text_static(lbl_bluetooth, LV_SYMBOL_BLUETOOTH);
    if (bat_per > 90) {
        lv_label_set_text_static(lbl_battery, LV_SYMBOL_BATTERY_FULL);
    }
    else if (bat_per > 70) {
        lv_label_set_text_static(lbl_battery, LV_SYMBOL_BATTERY_3);
    }
    else if (bat_per > 40) {
        lv_label_set_text_static(lbl_battery, LV_SYMBOL_BATTERY_2);
    }
    else if (bat_per > 10) {
        lv_label_set_text_static(lbl_battery, LV_SYMBOL_BATTERY_1);
    }
    else {
        lv_label_set_text_static(lbl_battery, LV_SYMBOL_BATTERY_EMPTY);
    }
    // lv_label_set_text_fmt(lbl_bat_lvl,      "%d%%", bat_per);

    const LOCATION * location = app_get_locatioin();
    if (location)
    {
        lv_label_set_text_static(lbl_location, location->city);
    }
    else
    {
        lv_label_set_text_static(lbl_location, "Unknown");
    }
    lv_label_set_text_static(lbl_update_tm, "");
}
