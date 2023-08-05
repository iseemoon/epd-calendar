#include <time.h>
#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "status_bar.h"
#include "ui_epd.h"
#include "../common/date.h"
#include "../cloud/app_cloud.h"
#include "../misc/app_nvs.h"


extern lv_font_t chinese_hei_16;
extern lv_font_t dseg_big;
extern lv_font_t weather_icon;

static const char * TAG = "DIGI";

static lv_obj_t * lbl_year = NULL;
static lv_obj_t * lbl_month = NULL;
static lv_obj_t * lbl_day = NULL;
static lv_obj_t * lbl_week = NULL;
static lv_obj_t * lbl_lunar_date = NULL;

static lv_obj_t * lbl_time = NULL;

static RTC_DATA_ATTR time_t full_upd_time = 0;

static void refresh_date(struct tm * date);
static void refresh_time(struct tm * date);

static void create_date_panel(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel, lv_pct(100), 60);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_gap(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(panel, 30, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_text_color(panel, lv_color_black(), 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_cross_place(panel, LV_FLEX_ALIGN_CENTER, 0);

    // 公历日期
    lv_obj_t * solar_panel = lv_obj_create(panel);
    lv_obj_set_size(solar_panel, lv_pct(100), lv_pct(50));
    lv_obj_set_style_pad_gap(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(solar_panel, lv_color_white(), 0);
    lv_obj_set_style_text_color(solar_panel, lv_color_black(), 0);
    lv_obj_set_flex_flow(solar_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(solar_panel, LV_FLEX_ALIGN_CENTER, 0);

    // 公历年
    lbl_year = lv_label_create(solar_panel);
    lv_obj_t * label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "年");

    // 公历月
    lbl_month = lv_label_create(solar_panel);
    label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "月");

    // 公历日
    lbl_day = lv_label_create(solar_panel);
    label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "日");

    // 星期
    lv_obj_t * space = lv_label_create(solar_panel);
    lv_label_set_text_static(space, "   ");
    lbl_week = lv_label_create(solar_panel);

    // 农历日期
    lbl_lunar_date = lv_label_create(panel);

    // 温湿度
    // lbl_lunar_date = lv_label_create(panel);
    // lv_obj_set_pos(lbl_lunar_date, 30, 40);
    // lv_obj_set_style_text_font(lbl_lunar_date, &chinese_hei_16, 0);
}

static void create_time_panel(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel, lv_pct(100), lv_pct(35));
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_text_color(panel, lv_color_black(), 0);

    lbl_time = lv_label_create(panel);
    lv_obj_set_style_text_font(lbl_time, &dseg_big, 0);
}

static void create_weather_panel(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel, lv_pct(100), lv_pct(40));
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(panel, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(panel, 30, 0);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_text_color(panel, lv_color_black(), 0);

    char key[12];
    WEATHER weather;
    for (int i=0; ; ++i)
    {
        snprintf(key, sizeof(key), "w%d", i);
        if (!nvs_read_blob(NV_NAMESPACE_WEATHER, key, &weather, sizeof(WEATHER))) {
            break;
        }

        lv_obj_t * p1 = lv_label_create(panel);
        if (i > 0) lv_obj_set_style_pad_left(p1, 6, 0);
        lv_label_set_text_static(p1, "");
        lv_obj_set_flex_flow(p1, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_flex_cross_place(p1, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_t * lbl_date = lv_label_create(p1);
        lv_obj_t * lbl_weather = lv_label_create(p1);
        lv_obj_set_style_text_font(lbl_weather, &weather_icon, 0);
        lv_obj_set_style_text_color(lbl_weather, lv_color_make(0xFF, 0, 0), 0);
        lv_obj_t * lbl_text = lv_label_create(p1);
        lv_obj_t * lbl_temp = lv_label_create(p1);

        if (i==0) {
            lv_label_set_text_static(lbl_date, "今天");
        }
        else {
            lv_label_set_text_fmt(lbl_date, "%d月%d日", weather.month, weather.day);
        }
        switch (weather.day_code)
        {
        case 0:
            lv_label_set_text_static(lbl_weather, "\xEF\x84\x81");  // 晴
            break;
        case 1:
            lv_label_set_text_static(lbl_weather, "\xEF\x84\x82");  // 多云
            break;
        case 2:
            lv_label_set_text_static(lbl_weather, "\xEF\x84\x85");  // 阴
            break;
        case 3:     // 阵雨
            lv_label_set_text_static(lbl_weather, "\xEF\x84\x8A");
            break;
        case 4:     // 雷阵雨
            lv_label_set_text_static(lbl_weather, "\xEF\x84\x8C");
            break;
        case 5:  
        case 6: 
        case 7:     // 小雨
            lv_label_set_text_static(lbl_weather, "\xEF\x84\x8F");
            break;
        case 8:     // 中雨
            lv_label_set_text_static(lbl_weather, "\xEF\x84\x90");
            break;
        case 9:     // 大雨
            lv_label_set_text_static(lbl_weather, "\xEF\x84\x91");
            break;
        default:
            break;
        }
        
        lv_label_set_text(lbl_text, weather.day_text);
        lv_label_set_text_fmt(lbl_temp, "%d~%d°", weather.temp_min, weather.temp_max); // °C
    }
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
    lv_label_set_text_fmt(lbl_lunar_date, "农历%s%s", get_lunar_month_str(lunar_date.month, lunar_date.ext.is_leap_month),
        get_lunar_day_str(lunar_date.day));
}

lv_obj_t * create_dig_clock_scr()
{
    if (time(NULL) < 946684800)    // 2000年1月1日
    {
        return NULL;
    }

    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);

    create_status_bar(scr);
    create_date_panel(scr);
    create_time_panel(scr);

    set_flush_mode(FLUSH_FULL);

    return scr;
}


///////////////////////////////////////////////////////////

static void create_ui(struct _SCR_CONTEXT * ctx)
{
    ESP_LOGI(TAG, "create calendar scr enter");

    ctx->scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ctx->scr, lv_color_white(), 0);
    lv_obj_set_flex_flow(ctx->scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_text_font(ctx->scr, &chinese_hei_16, 0);
    lv_obj_set_style_flex_cross_place(ctx->scr, LV_FLEX_ALIGN_CENTER, 0);

    create_status_bar(ctx->scr);
    create_date_panel(ctx->scr);
    create_time_panel(ctx->scr);
    create_weather_panel(ctx->scr);

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

static void refresh_time(struct tm * date)
{
    lv_label_set_text_fmt(lbl_time, "%d:%02d", date->tm_hour, date->tm_min);
    if (date->tm_hour < 20) {
        lv_obj_align(lbl_time, LV_ALIGN_CENTER, -25, 0);
    }
    else {
        lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, 0);
    }
}

static void prepare(struct _SCR_CONTEXT * ctx, uint8_t prev_id)
{
    struct tm date, tmp;
    time_t now = time(NULL);
    localtime_r(&now, &date);
    localtime_r(&full_upd_time, &tmp);

    if (prev_id == ctx->scr_id && difftime(now, full_upd_time) < 600 && date.tm_mday == tmp.tm_mday)
    {
        set_flush_mode(FLUSH_PARTIAL_FAST);
        lv_disp_t * disp_refr = lv_disp_get_default();
        lv_obj_update_layout(disp_refr->act_scr);
        _lv_inv_area(NULL, NULL);
    }
    else
    {
        full_upd_time = now;
        set_flush_mode(FLUSH_FULL);
        refresh_status_bar();
        refresh_date(&date);
    }
    refresh_time(&date);
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

void init_digi_clock_ctx(struct _SCR_CONTEXT * ctx)
{
    ctx->scr = NULL;
    ctx->scr_id = SCR_ID_DIGI_CLK;
    ctx->user_data = NULL;
    ctx->delete_ctx = &delete_ctx;
    ctx->create_ui = &create_ui;
    ctx->discard_ui = &discard_ui;
    ctx->prepare = &prepare;
    ctx->draw_finished = &draw_finished;
    ctx->on_key_down = &on_key_down;
    ctx->on_key_up = &on_key_up;
}
