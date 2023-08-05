#include <time.h>
#include <stdio.h>
#include "lvgl.h"
#include "esp_log.h"
#include "esp_system.h"
#include "lvgl_calendar.h"
#include "status_bar.h"
#include "ui_epd.h"
#include "common/date.h"

extern lv_font_t chinese_hei_16;
extern lv_font_t font_dseg;

static const char * TAG = "CALENDAR";

static lv_obj_t * lbl_year = NULL;
static lv_obj_t * lbl_month = NULL;
static lv_obj_t * lbl_day = NULL;
static lv_obj_t * lbl_week = NULL;
static lv_obj_t * lbl_ganzhi = NULL;
static lv_obj_t * lbl_lunar_date = NULL;

static lv_obj_t * lbl_time = NULL;

static lv_timer_t * refresh_timer = NULL;

static RTC_DATA_ATTR time_t full_upd_time = 0;

static void refresh_date(struct tm * date);
static void refresh_time(struct tm * date);

static void create_date_panel(lv_obj_t * parent)
{
    /*Create a font*/
    // static lv_ft_info_t info;
    // /*FreeType uses C standard file system, so no driver letter is required.*/
    // info.name = "/littlefs/songti.ttf";
    // info.weight = 48;
    // info.style = FT_FONT_STYLE_BOLD;
    // // info.mem = NULL;
    // if(!lv_ft_font_init(&info)) {
    //     LV_LOG_ERROR("create failed.");
    // }

    // /*Create style with the new font*/
    // static lv_style_t style;
    // lv_style_init(&style);

    // lv_style_set_text_font(&style, info.font);
    // lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);
    // lv_style_set_bg_color(&style, lv_color_hex(0x000000));
    // lv_style_set_text_color(&style, lv_color_make(0, 0, 0xFF));

    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, 0, get_status_bar_height());
    lv_obj_set_size(panel, lv_pct(50), 74);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_gap(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(panel, &chinese_hei_16, 0);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_text_color(panel, lv_color_black(), 0);
    // lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * solar_panel = lv_obj_create(panel);
    lv_obj_set_pos(solar_panel, 30, 10);
    lv_obj_set_size(solar_panel, lv_pct(70), lv_pct(50));
    lv_obj_set_style_pad_gap(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(solar_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_color(solar_panel, lv_color_white(), 0);
    lv_obj_set_style_text_color(solar_panel, lv_color_black(), 0);

    /*Create a label with the new style*/
    lbl_year = lv_label_create(solar_panel);
    lv_obj_set_style_text_font(lbl_year, &chinese_hei_16, 0);
    lv_obj_t * label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "年");

    lbl_month = lv_label_create(solar_panel);
    lv_obj_set_style_text_font(lbl_month, &chinese_hei_16, 0);
    label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "月");

    lbl_day = lv_label_create(solar_panel);
    lv_obj_set_style_text_font(lbl_day, &chinese_hei_16, 0);
    label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "日");

    label = lv_label_create(solar_panel);   // column span
    lv_obj_set_size(label, 48, 0);
    lbl_week = lv_label_create(solar_panel);
    lv_obj_set_pos(lbl_week, 120, 10);

    lbl_lunar_date = lv_label_create(panel);
    lv_obj_set_pos(lbl_lunar_date, 30, 40);
    lv_obj_set_style_text_font(lbl_lunar_date, &chinese_hei_16, 0);

    lbl_ganzhi = lv_label_create(panel);
    lv_obj_set_style_text_font(lbl_ganzhi, &chinese_hei_16, 0);
    lv_obj_set_pos(lbl_ganzhi, 180, 40);
}

static void create_time_panel(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, lv_pct(50), get_status_bar_height());
    lv_obj_set_size(panel, lv_pct(50), 74);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_text_color(panel, lv_color_black(), 0);
    // lv_obj_set_style_text_font(panel, &lv_font_simsun_16_cjk, 0);

    lbl_time = lv_label_create(panel);
    // lv_obj_align(lbl_time, LV_ALIGN_TOP_RIGHT, -32, 5);
    lv_obj_align(lbl_time, LV_ALIGN_TOP_RIGHT, -120, 5);
    lv_obj_set_style_text_font(lbl_time, &font_dseg, 0);
}

static void refresh_date(struct tm * date)
{
    int32_t year  = date->tm_year + 1900;
    uint8_t month = date->tm_mon+1;
    uint8_t day   = date->tm_mday;
    lv_label_set_text_fmt(lbl_year, "%ld", year);
    lv_label_set_text_fmt(lbl_month, "%d", month);
    lv_label_set_text_fmt(lbl_day, "%d", day);
    lv_label_set_text_fmt(lbl_week, "星期%s", get_week_str(date->tm_wday));

    DATE lunar_date;
    get_lunar_date(year, month, day, &lunar_date);
    const char * animal = get_animal_str(lunar_date.year);
    const char * year_ganzhi = get_year_ganzhi_str(lunar_date.year);
    const char * month_ganzhi = get_month_ganzhi_str(lunar_date.year, lunar_date.month);
    const char * day_ganzhi = get_day_ganzhi_str(year, month, day);
    lv_label_set_text_fmt(lbl_lunar_date, "农历%s年%s%s", animal, get_lunar_month_str(lunar_date.month, lunar_date.ext.is_leap_month),
        get_lunar_day_str(lunar_date.day));
    lv_label_set_text_fmt(lbl_ganzhi, "%s年%s月%s日", year_ganzhi, month_ganzhi, day_ganzhi);
    
    // printf("\n%s%s年 %s月 %s日\n", year_ganzhi, animal, month_ganzhi, day_ganzhi);
    // printf("\n%d年 %d月 %d日\n", lunar_date.year, lunar_date.month, lunar_date.day);
}

static void refresh_time(struct tm * date)
{
    lv_label_set_text_fmt(lbl_time, "%02d:%02d", date->tm_hour, date->tm_min);
}

// static void refresh()
// {
//     struct tm date;
//     time_t now = time(NULL);
//     memcpy(&date, localtime(&now), sizeof(struct tm));

//     refresh_status_bar();
//     refresh_date(&date);
//     refresh_time(&date);

//     update_calender_date(date.tm_year + 1900, date.tm_mon+1, date.tm_mday);
// }

static void refr_timer_cb(struct _lv_timer_t * timer)
{
    if (timer->period < 60)
    {
        lv_timer_set_period(timer, 60000);
    }

    // refresh();
}

lv_obj_t * create_calendar_scr()
{
    if (time(NULL) < 946684800)    // 2000年1月1日
    {
        return NULL;
    }

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);

    create_status_bar(scr);
    create_date_panel(scr);
    create_time_panel(scr);
    lv_create_calender(scr, 20, 100, 760, 360);

    // refresh();

    set_flush_mode(FLUSH_FULL);

    return scr;
}

static void create_ui(struct _SCR_CONTEXT * ctx)
{
    ESP_LOGI(TAG, "create calendar scr enter");

    ctx->scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ctx->scr, lv_color_white(), 0);

    create_status_bar(ctx->scr);
    create_date_panel(ctx->scr);
    create_time_panel(ctx->scr);
    lv_create_calender(ctx->scr, 20, 100, 760, 360);

    ESP_LOGI(TAG, "create calendar scr leave");
}

static void delete_ctx(struct _SCR_CONTEXT * ctx)
{
    
}

static void discard_ui(struct _SCR_CONTEXT * ctx)
{
    if (ctx->scr)
    {
        lv_obj_del(ctx->scr);
        ctx->scr = NULL;
    }
}

static void prepare(struct _SCR_CONTEXT * ctx, uint8_t prev_id)
{
    struct tm date, tmp;
    time_t now = time(NULL);
    localtime_r(&now, &date);
    localtime_r(&full_upd_time, &tmp);

    if (prev_id == ctx->scr_id && difftime(now, full_upd_time) < 900 && date.tm_mday == tmp.tm_mday)
    {
        set_flush_mode(FLUSH_PARTIAL_FAST);
        lv_disp_t * disp_refr = lv_disp_get_default();
        lv_obj_update_layout(disp_refr->act_scr);
        _lv_inv_area(NULL, NULL);
        refresh_time(&date);
    }
    else
    {
        full_upd_time = now;
        set_flush_mode(FLUSH_FULL);
        refresh_status_bar();
        refresh_date(&date);
        refresh_time(&date);
        update_calender_date(date.tm_year + 1900, date.tm_mon+1, date.tm_mday);
    }
}

static void draw_finished(struct _SCR_CONTEXT * ctx)
{
    notify_exit();
}

static void on_key_down(struct _SCR_CONTEXT * ctx, uint32_t key)
{
    ESP_LOGI(TAG, "key %lu down", key);
}

static void on_key_up(struct _SCR_CONTEXT * ctx, uint32_t key)
{
    ESP_LOGI(TAG, "key %lu up", key);

    lv_group_t * group = lv_group_get_default();
    switch (key)
    {
    case LV_KEY_UP:
        lv_group_focus_prev(group);
        break;
    case LV_KEY_DOWN:
        lv_group_focus_next(group);
        break;
    case LV_KEY_RIGHT:
        lv_group_focus_next(group);
        break;
    case LV_KEY_LEFT:
        lv_group_focus_prev(group);
        break;
    case LV_KEY_ENTER:
        lv_obj_t * focus = lv_group_get_focused(group);
        if (focus)
        {
            lv_event_send(focus, LV_EVENT_CLICKED, NULL);
        }
        break;
    case LV_KEY_ESC:
        break;
    }
}

void init_calendar_ctx(struct _SCR_CONTEXT * ctx)
{
    ctx->scr = NULL;
    ctx->scr_id = SCR_ID_CALENDAR;
    ctx->user_data = NULL;
    ctx->delete_ctx = &delete_ctx;
    ctx->create_ui = &create_ui;
    ctx->discard_ui = &discard_ui;
    ctx->prepare = &prepare;
    ctx->draw_finished = &draw_finished;
    ctx->on_key_down = &on_key_down;
    ctx->on_key_up = &on_key_up;
}
