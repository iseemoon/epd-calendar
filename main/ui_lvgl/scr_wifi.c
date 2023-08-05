#include <string.h>
#include <time.h>
#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "status_bar.h"
#include "epd_panel_ops.h"
#include "ui_epd.h"
#include "../misc/app_nvs.h"
#include "../misc/app_wifi.h"
#include "../http/http_server.h"


extern lv_font_t chinese_hei_16;

extern void lv_indev_set_key_group(lv_group_t * group);

const char * TIPS1    = "1. 请使用手机或电脑连接以下WiFi热点";
// const char* HOST_NAME = AP_SSID;                 // 设置设备名
const char * TIPS2     = "2. 请使用手机扫描二维码或在浏览器中输入 #ff0000 %s# 进行配网";
const char* HOME_URL   = "http://192.168.4.1";

static const char * TAG = "WIFI";

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_KEY)
    {
        lv_group_t * group = lv_group_get_default();
        uint32_t key = lv_event_get_key(e);
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
        // case LV_KEY_ESC:
        //     lv_scr_load(scr_home);
        }
    }
    else if(code == LV_EVENT_CLICKED)
    {
        // const HOME_BTN * btn = (HOME_BTN *)e->user_data;
        // printf("button %ld Clicked\n", btn->id);
        // switch (btn->id)
        // {
        // case 1:
        //     break;
        // case 2:
        //     if (!scr_dig_clock) scr_dig_clock = create_dig_clock_scr();
        //     lv_scr_load(scr_dig_clock);
        //     break;
        // case 3:
        //     break;
        // case 4:
        //     if (!scr_calendar) scr_calendar = create_calendar_scr();
        //     lv_scr_load(scr_calendar);
        //     break;
        // case 5:
        //     break;
        // case 6:
        //     break;
        // case 7:
        //     break;
        // default:
        //     break;
        // }
    }
}

LV_IMG_DECLARE(img_reset);

static void create_panel(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel, lv_pct(70), lv_pct(80));
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_text_color(panel, lv_color_black(), 0);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    // lv_obj_set_style_border_color(panel, lv_color_make(0, 0, 0xff), 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_text_font(panel, &chinese_hei_16, 0);
    lv_obj_center(panel);

    lv_obj_t * label = lv_label_create(panel);
    lv_label_set_text_static(label, TIPS1);

    label = lv_label_create(panel);
    lv_label_set_text_fmt(label, "SSID: %s", get_ap_ssid());
    lv_obj_set_style_pad_left(label, 20, 0);

    label = lv_label_create(panel);
    lv_label_set_text_fmt(label, "密码:  %s", get_ap_password());
    lv_obj_set_style_pad_left(label, 20, 0);

    label = lv_label_create(panel);
    lv_label_set_recolor(label, true);
    lv_label_set_text_fmt(label, TIPS2, HOME_URL);
    lv_obj_set_style_pad_top(label, 8, 0);

    lv_obj_t * qr = lv_qrcode_create(panel, 120, lv_color_black(), lv_color_white());
    lv_qrcode_update(qr, HOME_URL, strlen(HOME_URL));
    lv_obj_set_style_pad_left(qr, 20, 0);

    lv_obj_t * footer = lv_obj_create(panel);
    lv_obj_set_size(footer, lv_pct(100), 42);
    lv_obj_set_style_pad_all(footer, 0, 0);
    lv_obj_set_style_bg_color(footer, lv_color_white(), 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_t * btn = lv_btn_create(footer);
    lv_obj_set_size(btn, 108, 42);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -20, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(btn, lv_color_white(), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_black(), 0);
    lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, NULL);
    lv_group_t * group = lv_group_get_default();
    lv_group_add_obj(group, btn);

    lv_obj_t * img = lv_img_create(btn);
    lv_img_set_src(img, &img_reset);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 6, 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
    lv_obj_set_style_img_recolor(img, lv_color_black(), 0);

    /*Create a label on the image button*/
    label = lv_label_create(btn);
    lv_label_set_text_static(label, "返回");
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 12, 4);
}

static void wifi_monitor_task(void *pvParameter)
{
    if (wait_for_wifi_config(UINT32_MAX))
    {
        
        notify_wifi_set();
    }

    vTaskDelete(NULL);
}

lv_obj_t * create_wifi_scr(lv_disp_t * disp)
{
    lv_obj_t * scr_wifi = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_wifi, lv_color_white(), 0);

    set_flush_mode(FLUSH_FULL);

    create_panel(scr_wifi);

    xTaskCreate(wifi_monitor_task, "monitor", 1024, NULL, tskIDLE_PRIORITY, NULL);

    app_wifi_init();

    wifi_init_softap();

    start_http_server("/fs/web");

    return scr_wifi;
}

static void create_ui(struct _SCR_CONTEXT * ctx)
{
    ESP_LOGI(TAG, "create wifi scr enter");

    ctx->scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ctx->scr, lv_color_white(), 0);

    create_panel(ctx->scr);

    ESP_LOGI(TAG, "create wifi scr leave");
}

static void delete_ctx(struct _SCR_CONTEXT * ctx)
{
    
}

static void discard_ui(struct _SCR_CONTEXT * ctx)
{
    if (ctx->scr)
    {
        ESP_LOGI(TAG, "discard wifi ui, deinit wifi.");
        app_wifi_deinit();
        lv_obj_del(ctx->scr);
        ctx->scr = NULL;
    }
}

static void prepare(struct _SCR_CONTEXT * ctx, uint8_t prev_id)
{
    set_flush_mode(FLUSH_FULL);

    app_wifi_init();

    wifi_init_softap();

    start_http_server("/fs/web");

    xTaskCreate(wifi_monitor_task, "monitor", 2048, NULL, tskIDLE_PRIORITY, NULL);
}

static void draw_finished(struct _SCR_CONTEXT * ctx)
{
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

void init_wifi_ctx(struct _SCR_CONTEXT * ctx)
{
    ctx->scr = NULL;
    ctx->scr_id = SCR_ID_WIFI;
    ctx->user_data = NULL;
    ctx->delete_ctx = &delete_ctx;
    ctx->create_ui = &create_ui;
    ctx->discard_ui = &discard_ui;
    ctx->prepare = &prepare;
    ctx->draw_finished = &draw_finished;
    ctx->on_key_down = &on_key_down;
    ctx->on_key_up = &on_key_up;
}