#pragma once
#include <stdio.h>
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCR_ID_HOME         0
#define SCR_ID_WIFI         1
#define SCR_ID_DIGI_CLK     2
#define SCR_ID_ANALOGUE_CLK 3
#define SCR_ID_CALENDAR     4
#define SCR_ID_GALLERY      5
#define SCR_ID_UPDATE       6
#define SCR_ID_RESET        99


typedef struct _SCR_CONTEXT {
    lv_obj_t * scr;
    uint32_t scr_id;
    void * user_data;
    void (*delete_ctx)(struct _SCR_CONTEXT * ctx);
    void (*create_ui)(struct _SCR_CONTEXT * ctx);
    void (*discard_ui)(struct _SCR_CONTEXT * ctx);
    void (*prepare)(struct _SCR_CONTEXT * ctx, uint8_t prev_id);
    void (*draw_finished)(struct _SCR_CONTEXT * ctx);
    void (*on_key_down)(struct _SCR_CONTEXT * ctx, uint32_t key);
    void (*on_key_up)(struct _SCR_CONTEXT * ctx, uint32_t key);
} SCR_CONTEXT;


void init_epd_panel();

void set_flush_mode(EPD_FLUSH_MODE mode);

void init_lvgl();

bool ui_loop();

void notify_wifi_set();

void notify_exit();

void show_screen_by_id(uint8_t id);

void show_msg_scr(const char * msg);

#ifdef __cplusplus
}
#endif
