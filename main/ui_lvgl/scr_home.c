#include <time.h>
#include <stdio.h>
#include "lvgl.h"
#include "epd_panel_ops.h"
#include "status_bar.h"
#include "esp_log.h"
#include "esp_system.h"
#include "ui_epd.h"
#include "../common/date.h"
#include "../misc/app_nvs.h"

static const char * TAG = "HOME";

extern lv_font_t chinese_hei_16;

// status bar
#define FW_VER              "Ver: 1.0"

lv_obj_t * create_home_scr();
extern lv_obj_t * create_dig_clock_scr();
extern lv_obj_t * create_wifi_scr(lv_disp_t * disp);

extern void lv_indev_set_key_group(lv_group_t * group);

LV_IMG_DECLARE(img_calendar);
LV_IMG_DECLARE(img_calendar1);
LV_IMG_DECLARE(img_clock);
LV_IMG_DECLARE(img_dig_colck);
LV_IMG_DECLARE(img_gallery);
LV_IMG_DECLARE(img_reset);
LV_IMG_DECLARE(img_upgrade);
LV_IMG_DECLARE(img_wifi);

typedef struct _HOME_BTN {
    uint32_t id;
    const char * name;
    const lv_img_dsc_t * img;
} HOME_BTN;

static const HOME_BTN g_btns[] = {
    {SCR_ID_WIFI,         "WiFi配网", &img_wifi},
    {SCR_ID_DIGI_CLK,     "数字时钟",  &img_dig_colck},
    {SCR_ID_ANALOGUE_CLK, "模拟时钟",  &img_clock},
    {SCR_ID_CALENDAR,     "日历界面",  &img_calendar},
    {SCR_ID_GALLERY,      "相框模式",  &img_gallery},
    {SCR_ID_UPDATE,       "在线升级",  &img_upgrade},
    {SCR_ID_RESET,        "恢复重置",  &img_reset},
};

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {50, 50, 50, LV_GRID_TEMPLATE_LAST};
static const char * MSG_BOX_BTNS[] = {"取消", "确定", ""};

static lv_obj_t * scr_home        = NULL;

static void event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * msgbox = lv_event_get_current_target(e);
 
    if ((code == LV_EVENT_VALUE_CHANGED) && (msgbox != NULL))
    {
        uint16_t idx = lv_msgbox_get_active_btn(msgbox);
        if(idx == 0)
        {
            lv_group_remove_obj(msgbox);
            lv_msgbox_close(msgbox);
        }
        else
        {
            ESP_LOGI(TAG, "==> Clear NV entries...");
            nvs_clear_all();
            ESP_LOGI(TAG, "==> Restart now...");
            esp_restart();
        }
    }
}

static void reset_to_default(lv_obj_t * parent)
{
    set_flush_mode(FLUSH_PARTIAL_FAST);
    lv_obj_t * mbox = lv_msgbox_create(parent, "重置", "确定要恢复出厂设置吗?", MSG_BOX_BTNS, false);
    lv_obj_set_style_bg_color(mbox, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(mbox, &chinese_hei_16, 0);
    lv_obj_set_style_text_color(mbox, lv_color_black(), 0);
    lv_obj_add_event_cb(mbox, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox);
    lv_obj_t * btns = lv_msgbox_get_btns(mbox);
    lv_obj_set_style_bg_color(btns, lv_color_white(), LV_PART_MAIN);
    lv_btnmatrix_set_selected_btn(btns, 0);
    lv_group_add_obj(lv_group_get_default(), btns);
    lv_group_focus_obj(btns);
}

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    // ESP_LOGI(TAG, "home event %d", code);

    if (code == LV_EVENT_KEY)
    {
        set_flush_mode(FLUSH_PARTIAL_FAST);
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
            lv_refr_now(NULL);
            break;
        case LV_KEY_LEFT:
            lv_group_focus_prev(group);
            lv_refr_now(NULL);
            break;
        case LV_KEY_ESC:
            show_screen_by_id(0);
        }
    }
    else if(code == LV_EVENT_CLICKED)
    {
        const HOME_BTN * btn = (HOME_BTN *)e->user_data;
        const SCR_CONTEXT * ctx = (SCR_CONTEXT *)e->param;
        ESP_LOGI(TAG, "==> button %lu Clicked", btn->id);
        nvs_set_screen(btn->id);
        if (btn->id == SCR_ID_RESET)
        {
            reset_to_default(ctx->scr);
        }
        else
        {
            show_screen_by_id(btn->id);
        }
    }
}

static void create_panel(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel, lv_pct(70), 240);
    lv_obj_set_layout(panel, LV_LAYOUT_GRID);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_grid_dsc_array(panel, col_dsc, row_dsc);
    lv_obj_set_style_bg_color(panel, lv_color_white(), LV_PART_MAIN);
    // lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(panel, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_row(panel, 24, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(panel, &chinese_hei_16, 0);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 20);

    lv_group_t * group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_set_key_group(group);
    for (int i=0; i<sizeof(g_btns)/sizeof(HOME_BTN); ++i)
    {
        lv_obj_t * btn = lv_btn_create(panel);
        lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, i%4, 1,
                                  LV_GRID_ALIGN_STRETCH, i/4, 1);
        lv_obj_set_style_bg_color(btn, lv_color_white(), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_black(), 0);
        lv_obj_set_style_shadow_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
        lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, &g_btns[i]);
        lv_group_add_obj(group, btn);

        lv_obj_t * img = lv_img_create(btn);
        lv_img_set_src(img, g_btns[i].img);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 6, 0);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
        lv_obj_set_style_img_recolor(img, lv_color_black(), 0);

        /*Create a label on the image button*/
        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text_static(label, g_btns[i].name);
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 18, 4);
    }

    lv_obj_t * label = lv_label_create(panel);
    lv_label_set_text_static(label, FW_VER);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_CENTER, 0, 4,
                                  LV_GRID_ALIGN_END, 2, 1);
}

static void create_ui(struct _SCR_CONTEXT * ctx)
{
    ESP_LOGI(TAG, "create home scr enter");

    ctx->scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ctx->scr, lv_color_white(), 0);
    create_panel(ctx->scr);

    ESP_LOGI(TAG, "create home scr leave");
}

static void delete_ctx(struct _SCR_CONTEXT * ctx)
{
    
}

static void discard_ui(struct _SCR_CONTEXT * ctx)
{
    // if (ctx->scr)
    // {
    //     lv_obj_del(ctx->scr);
    //     ctx->scr = NULL;
    // }
}

static void prepare(struct _SCR_CONTEXT * ctx, uint8_t prev_id)
{
    set_flush_mode(FLUSH_FULL);
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
    static uint32_t _key;

    lv_group_t * group = lv_group_get_default();
    lv_obj_t * focus = lv_group_get_focused(group);
    if (focus)
    {
        if (key == LV_KEY_ENTER)
        {
            lv_event_send(focus, LV_EVENT_CLICKED, ctx);
        }
        else
        {
            _key = key;
            lv_event_send(focus, LV_EVENT_KEY, &_key);
        }
    }

    // switch (key)
    // {
    // case LV_KEY_UP:
    //     lv_group_focus_prev(group);
    //     break;
    // case LV_KEY_DOWN:
    //     lv_group_focus_next(group);
    //     break;
    // case LV_KEY_RIGHT:
    //     lv_group_focus_next(group);
    //     // set_flush_mode(FLUSH_PARTIAL_FAST);
    //     // lv_refr_now(NULL);
    //     break;
    // case LV_KEY_LEFT:
    //     lv_group_focus_prev(group);
    //     // set_flush_mode(FLUSH_PARTIAL_FAST);
    //     // lv_refr_now(NULL);
    //     break;
    // case LV_KEY_ENTER:
    //     lv_obj_t * focus = lv_group_get_focused(group);
    //     if (focus)
    //     {
    //         lv_event_send(focus, LV_EVENT_CLICKED, ctx);
    //     }
    //     break;
    // case LV_KEY_ESC:
    //     break;
    // }
}

void init_home_ctx(struct _SCR_CONTEXT * ctx)
{
    ctx->scr = NULL;
    ctx->scr_id = SCR_ID_HOME;
    ctx->user_data = NULL;
    ctx->delete_ctx = &delete_ctx;
    ctx->create_ui = &create_ui;
    ctx->discard_ui = &discard_ui;
    ctx->prepare = &prepare;
    ctx->draw_finished = &draw_finished;
    ctx->on_key_down = &on_key_down;
    ctx->on_key_up = &on_key_up;
}
