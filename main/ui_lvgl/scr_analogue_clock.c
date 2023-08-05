#include <time.h>
#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "status_bar.h"
#include "ui_epd.h"
#include "../common/date.h"


extern lv_font_t chinese_hei_16;
extern lv_font_t dseg_big;
LV_IMG_DECLARE(clock_bkg)
LV_IMG_DECLARE(hour_hand)
LV_IMG_DECLARE(minute_hand)

static const char * TAG = "CLOCK";

static lv_obj_t * lbl_year = NULL;
static lv_obj_t * lbl_month = NULL;
static lv_obj_t * lbl_day = NULL;
static lv_obj_t * lbl_week = NULL;
static lv_obj_t * lbl_lunar_date = NULL;

static lv_obj_t * img_clock = NULL;
static lv_obj_t * clk_meter = NULL;
static lv_meter_indicator_t * indic_min;
static lv_meter_indicator_t * indic_hour;
static lv_obj_t * img_hour = NULL;
static lv_obj_t * img_minute = NULL;

static RTC_DATA_ATTR time_t full_upd_time = 0;

static void refresh_date(struct tm * date);
static void refresh_time(struct tm * date);

static void create_date_panel(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, 0, lv_pct(60));
    lv_obj_set_size(panel, lv_pct(100), lv_pct(40));
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_gap(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(panel, &chinese_hei_16, 0);
    lv_obj_set_style_bg_color(panel, lv_color_black(), 0);
    lv_obj_set_style_text_color(panel, lv_color_make(0, 0, 0xFF), 0);

    // 公历日期
    lv_obj_t * solar_panel = lv_obj_create(panel);
    lv_obj_set_pos(solar_panel, 30, 10);
    lv_obj_set_size(solar_panel, lv_pct(70), lv_pct(50));
    lv_obj_set_style_pad_gap(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(solar_panel, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(solar_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_color(solar_panel, lv_color_black(), 0);
    lv_obj_set_style_text_color(solar_panel, lv_color_make(0, 0, 0xFF), 0);

    // 公历年
    lbl_year = lv_label_create(solar_panel);
    lv_obj_set_style_text_font(lbl_year, &chinese_hei_16, 0);
    lv_obj_t * label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "年");

    // 公历月
    lbl_month = lv_label_create(solar_panel);
    lv_obj_set_style_text_font(lbl_month, &chinese_hei_16, 0);
    label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "月");

    // 公历日
    lbl_day = lv_label_create(solar_panel);
    lv_obj_set_style_text_font(lbl_day, &chinese_hei_16, 0);
    label = lv_label_create(solar_panel);
    lv_label_set_text_static(label, "日");

    // 星期
    lbl_week = lv_label_create(solar_panel);
    lv_obj_set_pos(lbl_week, 120, 10);

    // 农历日期
    lbl_lunar_date = lv_label_create(panel);
    lv_obj_set_pos(lbl_lunar_date, 30, 40);
    lv_obj_set_style_text_font(lbl_lunar_date, &chinese_hei_16, 0);

    // 温湿度
    // lbl_lunar_date = lv_label_create(panel);
    // lv_obj_set_pos(lbl_lunar_date, 30, 40);
    // lv_obj_set_style_text_font(lbl_lunar_date, &chinese_hei_16, 0);
}

static void create_time_panel(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, 0, get_status_bar_height());
    lv_obj_set_size(panel, lv_pct(100), lv_pct(90));
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_text_color(panel, lv_color_black(), 0);

    img_clock = lv_img_create(panel);
    lv_img_set_src(img_clock, &clock_bkg);
    lv_obj_set_style_bg_color(img_clock, lv_color_white(), 0);
    lv_obj_align(img_clock, LV_ALIGN_CENTER, 0, 0);

    // clk_meter = lv_meter_create(panel);
    // lv_obj_set_size(clk_meter, 240, 240);
    // lv_obj_set_style_bg_color(clk_meter, lv_color_white(), 0);
    // lv_obj_set_style_border_width(clk_meter, 0, LV_PART_MAIN);
    // lv_obj_center(clk_meter);

    // lv_meter_scale_t * scale_min = lv_meter_add_scale(clk_meter);
    // // lv_meter_set_scale_ticks(clk_meter, scale_min, 61, 1, 10, lv_palette_main(LV_PALETTE_BLUE));
    // lv_meter_set_scale_range(clk_meter, scale_min, 0, 60, 360, 270);

    // /*Create another scale for the hours. It's only visual and contains only major ticks*/
    // lv_meter_scale_t * scale_hour = lv_meter_add_scale(clk_meter);
    // // lv_meter_set_scale_ticks(clk_meter, scale_hour, 12, 0, 0, lv_palette_main(LV_PALETTE_BLUE));               /*12 ticks*/
    // // lv_meter_set_scale_major_ticks(clk_meter, scale_hour, 1, 2, 20, lv_color_black(), 10);    /*Every tick is major*/
    // lv_meter_set_scale_range(clk_meter, scale_hour, 1, 12, 330, 300);       /*[1..12] values in an almost full circle*/

    // indic_min = lv_meter_add_needle_img(clk_meter, scale_min, &minute_hand, 5, 5);
    // indic_hour = lv_meter_add_needle_img(clk_meter, scale_min, &hour_hand, 5, 5);

    img_hour = lv_img_create(img_clock);
    lv_img_set_src(img_hour, &hour_hand);
    lv_obj_set_pos(img_hour, 189, 194 - 75);

    img_minute = lv_img_create(img_clock);
    lv_img_set_src(img_minute, &minute_hand);
    lv_obj_set_pos(img_minute, 189, 194 - 115);

    lv_img_header_t info;
    lv_img_decoder_get_info(&clock_bkg, &info);
    lv_coord_t cx = info.w / 2;
    lv_coord_t cy = info.h / 2;

    ESP_LOGI(TAG, "clock center: %d, %d", cx, cy);

    lv_img_set_pivot(img_hour, 5, 75);
    lv_img_set_pivot(img_minute, 5, 115);
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

lv_obj_t * create_anal_clock_scr()
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

    create_status_bar(ctx->scr);
    // create_date_panel(ctx->scr);
    create_time_panel(ctx->scr);
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
    ESP_LOGI(TAG, "time: %d:%02d", date->tm_hour, date->tm_min);
    lv_img_set_angle(img_hour, date->tm_hour * 300 + date->tm_min * 5);
    lv_img_set_angle(img_minute, date->tm_min * 60);
    lv_obj_invalidate(img_clock);

    // lv_meter_set_indicator_end_value(clk_meter, indic_min, (date->tm_min + 45) % 60);
    // lv_meter_set_indicator_end_value(clk_meter, indic_hour, (date->tm_hour + 9) % 12 * 5 + date->tm_min / 12);
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
        // refresh_date(&date);
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

void init_analogue_clock_ctx(struct _SCR_CONTEXT * ctx)
{
    ctx->scr = NULL;
    ctx->scr_id = SCR_ID_ANALOGUE_CLK;
    ctx->user_data = NULL;
    ctx->delete_ctx = &delete_ctx;
    ctx->create_ui = &create_ui;
    ctx->discard_ui = &discard_ui;
    ctx->prepare = &prepare;
    ctx->draw_finished = &draw_finished;
    ctx->on_key_down = &on_key_down;
    ctx->on_key_up = &on_key_up;
}
