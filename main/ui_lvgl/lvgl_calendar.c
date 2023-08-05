/**
 * @file lvgl_calendar.c
 *
 */
/*********************
 *      INCLUDES
 *********************/
#include "lvgl_calendar.h"
#include "../common/date.h"
#include "../common/solar_term.h"
#include <stdlib.h>
#include <stdio.h>
// #include "esp_log.h"

extern lv_font_t chinese_hei_16;

/*********************
 *      DEFINES
 *********************/
#define LV_CALENDAR_CTRL_TODAY      LV_BTNMATRIX_CTRL_CUSTOM_1
#define LV_CALENDAR_CTRL_HIGHLIGHT  LV_BTNMATRIX_CTRL_CUSTOM_2

#define WEEK_HEIGHT  32
#define TAG   "CALENDAR"

#define CLR_MAIN_BKG   lv_color_white()
#define CLR_WEEK_BKG   lv_color_black()
#define CLR_WEEK_TXT   lv_color_white()
#define CLR_DAY0_TXT   lv_color_make(0xff, 0, 0)
#define CLR_DAY1_TXT   lv_color_black()

/**********************
 *      TYPEDEFS
 **********************/
typedef struct _LV_DAY_ITEM {
    lv_obj_t * cont;
    lv_obj_t * lbl_solar;
    lv_obj_t * lbl_lunar;
} LV_DAY_ITEM;

/**********************
 *  STATIC PROTOTYPES
 **********************/
// static void my_calendar_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
// static void draw_part_begin_event_cb(lv_event_t * e);
// static void highlight_update(lv_obj_t * calendar);

/**********************
 *  STATIC VARIABLES
 **********************/
// const lv_obj_class_t my_calendar_class = {
//     .constructor_cb = my_calendar_constructor,
//     .width_def = (LV_DPI_DEF * 3) / 2,
//     .height_def =(LV_DPI_DEF * 3) / 2,
//     .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
//     .instance_size = sizeof(lv_calendar_t),
//     .base_class = &lv_btnmatrix_class
// };
static LV_DAY_ITEM lv_day_items[31];
static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static lv_coord_t row_dsc[] = {WEEK_HEIGHT, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void update_calender_date(int32_t year, uint8_t month, uint8_t day)
{
    DATE lunar_date;
    DATE t1_date, t2_date;
    const char * t1 = get_day_of_solar_term(year, (month-1) * 2, &t1_date);
    const char * t2 = get_day_of_solar_term(year, (month-1) * 2 + 1, &t2_date);
    // printf("\n%d/%d/%d: %s, %d/%d/%d: %s\n", t1_date.year, t1_date.month, t1_date.day, t1,
    //     t2_date.year, t2_date.month, t2_date.day, t2);

    uint8_t month_days  = get_month_days(year, month);
    uint8_t week_of_day1 = get_day_of_week(year, month, 1);
    // printf(">>>> month_days: %d, day_of_week: %d\n", month_days, week_of_day1);

    uint8_t row = 1;
    uint8_t col = week_of_day1;
    for (uint8_t i = 1; i <= month_days; ++i)
    {
        LV_DAY_ITEM * item = &lv_day_items[i-1];
        if (i > 28 && lv_obj_has_flag(item->cont, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_clear_flag(item->cont, LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_set_grid_cell(item->cont, LV_GRID_ALIGN_STRETCH, col, 1,
                                  LV_GRID_ALIGN_STRETCH, row, 1);
        if (i == day)
        {
            lv_obj_set_style_border_width(item->cont, 2, LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_border_width(item->cont, 0, LV_PART_MAIN);
        }

        // 显示公历
        lv_label_set_text_fmt(item->lbl_solar, "%d", i);
        lv_obj_set_style_text_color(item->lbl_solar, col == 0 || col == 6 ? CLR_DAY0_TXT : CLR_DAY1_TXT, 0);

        // 显示节日，如果节日为空的话，则显示农历
        get_lunar_date(year, month, i, &lunar_date);
        // printf(">>>> lunar_date: %d/%d/%d\n", lunar_date.year, lunar_date.month, lunar_date.day);

        const char * lun_festival  = get_lunar_festival(&lunar_date);
        const char * gre_festival  = get_gre_festival(month, i);
        const char * week_festival = get_week_festival(month, i, month_days, week_of_day1);
        if (lun_festival)
        {
            lv_label_set_text(item->lbl_lunar, lun_festival);
            lv_obj_set_style_text_color(item->lbl_lunar, CLR_DAY0_TXT, 0);
        }
        else if (gre_festival)
        {
            lv_label_set_text(item->lbl_lunar, gre_festival);
            lv_obj_set_style_text_color(item->lbl_lunar, CLR_DAY0_TXT, 0);
        }
        else if (week_festival)
        {
            lv_label_set_text(item->lbl_lunar, week_festival);
            lv_obj_set_style_text_color(item->lbl_lunar, CLR_DAY0_TXT, 0);
        }
        else
        {
            lv_obj_set_style_text_color(item->lbl_lunar, CLR_DAY1_TXT, 0);

            if (t1_date.day == i)
            {
                lv_label_set_text(item->lbl_lunar, t1);
            }
            else if (t2_date.day == i)
            {
                lv_label_set_text(item->lbl_lunar, t2);
            }
            else if (t2_date.day == 1)
            {
                lv_label_set_text(item->lbl_lunar, get_lunar_month_str(lunar_date.month, lunar_date.ext.is_leap_month));
            }
            else
            {
                lv_label_set_text(item->lbl_lunar, get_lunar_day_str(lunar_date.day));
            }
        }

        if (++col > 6)
        {
            col = 0;
            ++row;
        }
    }

    for (uint8_t i = month_days; i < 31; ++i)
    {
        lv_obj_add_flag(lv_day_items[i].cont, LV_OBJ_FLAG_HIDDEN);
    }
}

void lv_create_calender(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, size_t w, size_t h)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, w, h);
    lv_obj_set_layout(panel, LV_LAYOUT_GRID);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_grid_dsc_array(panel, col_dsc, row_dsc);
    lv_obj_set_style_bg_color(panel, CLR_MAIN_BKG, LV_PART_MAIN);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(panel, 4, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(panel, &chinese_hei_16, 0);

    lv_obj_t * label;
    lv_obj_t * obj;
    for (int i=0; i<7; ++i)
    {
        obj = lv_obj_create(panel);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, i, 1,
                                  LV_GRID_ALIGN_STRETCH, 0, 1);
        lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(obj, CLR_WEEK_BKG, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_border_width(obj, 0, 0);

        label = lv_label_create(obj);
        lv_label_set_text(label, get_week_str(i));
        lv_obj_set_style_text_color(label, CLR_WEEK_TXT, 0);
        lv_obj_center(label);
    }

    for (int i=0; i<31; ++i)
    {
        obj = lv_obj_create(panel);
        lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_radius(obj, 6, 0);
        lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
        lv_obj_set_style_border_color(obj, CLR_DAY0_TXT, LV_PART_MAIN);

        lv_day_items[i].cont = obj;
        lv_day_items[i].lbl_solar = lv_label_create(obj);
        lv_obj_align(lv_day_items[i].lbl_solar, LV_ALIGN_TOP_MID, 0, 4);

        lv_day_items[i].lbl_lunar = lv_label_create(obj);
        lv_obj_set_style_text_font(lv_day_items[i].lbl_lunar, &chinese_hei_16, 0);
        lv_obj_align(lv_day_items[i].lbl_lunar, LV_ALIGN_BOTTOM_MID, 0, 0);
    }
}


// lv_obj_t * my_calendar_create(lv_obj_t * parent)
// {
//     LV_LOG_INFO("begin")
//     lv_obj_t * obj = lv_obj_class_create_obj(&my_calendar_class, parent);
//     lv_obj_class_init_obj(obj);
//     return obj;
// }


// custom_calendar_t *create_custom_calendar(lv_obj_t *parent)
// {
//     custom_calendar_t *custom_calendar = malloc(sizeof(custom_calendar_t));
//     custom_calendar->parent = parent;

//     // 创建日历部件
//     custom_calendar->calendar = lv_obj_create(parent);
//     lv_obj_set_size(custom_calendar->calendar, lv_pct(100), lv_pct(80));
//     lv_obj_align(custom_calendar->calendar, LV_ALIGN_CENTER, 0, 0);

//     // 设置星期栏样式
//     lv_obj_t *week_bar = lv_obj_create(custom_calendar->calendar);
//     lv_obj_set_size(week_bar, lv_pct(100), 32);
//     lv_obj_set_style_bg_color(week_bar, lv_color_white(), LV_PART_MAIN);
//     lv_obj_set_style_text_color(week_bar, lv_color_make(0, 0, 0xFF), LV_PART_MAIN);
//     // lv_obj_set_style_text_font(week_bar, &lv_font_montserrat_16, LV_PART_MAIN);
//     lv_obj_set_style_text_align(week_bar, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
//     lv_obj_align(week_bar, LV_ALIGN_TOP_MID, 0, 0);

//     // 设置星期栏文本
//     char *week_days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
//     lv_coord_t day_width = lv_obj_get_width(custom_calendar->calendar) / 7;
//     for (int i = 0; i < 7; i++) {
//         lv_obj_t *label = lv_label_create(week_bar);
//         lv_label_set_text(label, week_days[i]);
//         lv_obj_set_width(label, day_width);
//         lv_obj_align(label, LV_ALIGN_TOP_LEFT, i * day_width, 0);
//     }

//     // 设置日期栏样式
//     lv_obj_t *date_bar = lv_obj_create(custom_calendar->calendar);
//     lv_obj_set_size(date_bar, lv_pct(100), lv_pct(80) - 32); // 80为整体高度，32为星期栏高度
//     lv_obj_align(date_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
//     lv_obj_set_style_bg_color(date_bar, lv_color_black(), LV_PART_MAIN);

//     // 设置日期文本样式
//     lv_obj_t *date_label = lv_label_create(date_bar);
//     // lv_obj_set_style_text_font(date_label,  &lv_font_montserrat_16, LV_PART_MAIN);
//     lv_obj_set_style_text_color(date_label, lv_color_make(0, 0, 0xFF), LV_PART_MAIN);
//     lv_obj_set_style_text_line_space(date_label, 4, LV_PART_MAIN);

//     // 获取当前日期
//     lv_calendar_date_t today = {
//         .year = 2023,
//         .month = 5,
//         .day = 19,
//     };

//     // 获取当前月份的天数
//     int days_in_month = 31;//lv_calendar_get_month_length(today.year, today.month);

//     // 计算当前月份的起始日期的星期几
//     int start_day_of_week = 3;//lv_calendar_get_day_of_week(today.year, today.month, 1);

//     // 计算日期格子的宽度和高度
//     lv_coord_t day_cell_width = lv_obj_get_width(date_bar) / 7;
//     lv_coord_t day_cell_height = lv_obj_get_height(date_bar) / (days_in_month / 7 + 1); // +1 是为了防止日期栏高度不够

//     // 创建日期格子
//     for (int i = 0; i < days_in_month; i++) {
//         // 计算日期格子的行列索引
//         int row = (start_day_of_week + i) / 7;
//         int col = (start_day_of_week + i) % 7;

//         // 创建日期格子
//         lv_obj_t *cell = lv_obj_create(date_bar);
//         lv_obj_set_size(cell, day_cell_width, day_cell_height);
//         lv_obj_align(cell, LV_ALIGN_TOP_LEFT, col * day_cell_width, row * day_cell_height);

//         // 设置日期格子文本
//         lv_obj_t *label = lv_label_create(cell);
//         lv_label_set_text_fmt(label, "%d\n阴历", i + 1);
//         // lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);
//         lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
//         lv_obj_set_style_text_line_space(label, 4, LV_PART_MAIN);

//         // 如果是当天的日期，设置红色边框
//         if (today.day == i + 1) {
//             lv_obj_set_style_border_color(cell, lv_color_make(0xff, 0, 0), LV_PART_MAIN);
//             lv_obj_set_style_border_width(cell, 2, LV_PART_MAIN);
//         }
//     }

//     // 设置日历的事件回调函数
//     // lv_obj_set_event_cb(custom_calendar->calendar, calendar_event_cb);

//     return custom_calendar;
// }

// void lv_create_calender(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, size_t w, size_t h, int32_t year, uint8_t month, uint8_t day)
// {
//     DATE lunar_date;
//     DATE solar_date = {
//         .year  = year,
//         .month = month,
//     };

//     lv_obj_t * panel = lv_obj_create(parent);
//     lv_obj_set_pos(panel, x, y);
//     lv_obj_set_size(panel, w, h);
//     lv_obj_set_layout(panel, LV_LAYOUT_GRID);
//     lv_obj_set_style_radius(panel, 0, 0);
//     lv_obj_set_grid_dsc_array(panel, col_dsc, row_dsc);
//     lv_obj_set_style_bg_color(panel, CLR_MAIN_BKG, LV_PART_MAIN);
//     lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
//     lv_obj_set_style_pad_row(panel, 0, LV_PART_MAIN);
//     lv_obj_set_style_pad_column(panel, 0, LV_PART_MAIN);
//     lv_obj_set_style_text_font(panel, &chinese_hei_16, 0);

//     lv_obj_t * label;
//     lv_obj_t * obj;
//     for (int i=0; i<7; ++i)
//     {
//         obj = lv_btn_create(panel);
//         lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, i, 1,
//                                   LV_GRID_ALIGN_STRETCH, 0, 1);
//         lv_obj_set_style_bg_color(obj, CLR_WEEK_BKG, 0);
//         lv_obj_set_style_radius(obj, 0, 0);

//         label = lv_label_create(obj);
//         lv_label_set_text(label, get_week_str(i));
//         lv_obj_set_style_text_color(label, CLR_WEEK_TXT, 0);
//         lv_obj_center(label);
//     }

//     DATE t1_date, t2_date;
//     const char * t1 = get_day_of_solar_term(year, (month-1) * 2, &t1_date);
//     const char * t2 = get_day_of_solar_term(year, (month-1) * 2 + 1, &t2_date);
//     // printf("\n%d/%d/%d: %s, %d/%d/%d: %s\n", t1_date.year, t1_date.month, t1_date.day, t1,
//     //     t2_date.year, t2_date.month, t2_date.day, t2);

//     uint8_t month_days  = get_month_days(year, month);
//     uint8_t week_of_day1 = get_day_of_week(year, month, 1);
//     // printf(">>>> month_days: %d, day_of_week: %d\n", month_days, week_of_day1);
//     uint8_t row = 1;
//     uint8_t col = week_of_day1;
//     for (uint8_t i = 1; i <= month_days; ++i)
//     {
//         obj = lv_btn_create(panel);
//         lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1,
//                                   LV_GRID_ALIGN_STRETCH, row, 1);
//         lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN);
//         lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
//         lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN);
//         lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
//         lv_obj_set_style_pad_gap(obj, 0, LV_PART_MAIN);
//         lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN);
//         lv_obj_set_style_pad_bottom(obj, 5, LV_PART_MAIN);
//         if (i == day)
//         {
//             lv_obj_set_style_radius(obj, 6, 0);
//             lv_obj_set_style_outline_color(obj, CLR_DAY0_TXT, LV_PART_MAIN);
//             lv_obj_set_style_outline_width(obj, 1, LV_PART_MAIN);
//         }

//         // 显示公历
//         label = lv_label_create(obj);
//         lv_label_set_text_fmt(label, "%d", i);
//         lv_obj_set_style_text_color(label, col == 0 || col == 6 ? CLR_DAY0_TXT : CLR_DAY1_TXT, 0);
//         lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

//         // 显示节日，如果节日为空的话，则显示农历
//         get_lunar_date(year, month, i, &lunar_date);

//         label = lv_label_create(obj);
//         lv_obj_set_style_text_font(label, &chinese_hei_16, 0);
//         lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);

//         // printf(">>>> lunar_date: %d/%d/%d\n", lunar_date.year, lunar_date.month, lunar_date.day);

//         const char * lun_festival  = get_lunar_festival(&lunar_date);
//         const char * gre_festival  = get_gre_festival(month, i);
//         const char * week_festival = get_week_festival(month, i, month_days, week_of_day1);
//         if (lun_festival)
//         {
//             lv_label_set_text(label, lun_festival);
//             lv_obj_set_style_text_color(label, CLR_DAY0_TXT, 0);
//         }
//         else if (gre_festival)
//         {
//             lv_label_set_text(label, gre_festival);
//             lv_obj_set_style_text_color(label, CLR_DAY0_TXT, 0);
//         }
//         else if (week_festival)
//         {
//             lv_label_set_text(label, week_festival);
//             lv_obj_set_style_text_color(label, CLR_DAY0_TXT, 0);
//         }
//         else
//         {
//             lv_obj_set_style_text_color(label, CLR_DAY1_TXT, 0);

//             if (t1_date.day == i)
//             {
//                 lv_label_set_text(label, t1);
//             }
//             else if (t2_date.day == i)
//             {
//                 lv_label_set_text(label, t2);
//             }
//             else if (t2_date.day == 1)
//             {
//                 lv_label_set_text(label, get_lunar_month_str(lunar_date.month, lunar_date.ext.is_leap_month));
//             }
//             else
//             {
//                 lv_label_set_text(label, get_lunar_day_str(&lunar_date));
//                 // lv_label_set_text_fmt(label, "%d", lunar_date.day);
//             }
//         }

//         if (++col > 6)
//         {
//             col = 0;
//             ++row;
//         }
//     }
// }
