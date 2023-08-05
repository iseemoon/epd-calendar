/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "internal/epd_panel_interface.h"
#include "epd_panel_io.h"
#include "epd_panel_ops.h"
#include "esp_log.h"
#include "esp_check.h"


#define EPD_GDEY075_MODEL   "GDEY075Z08"
#define EPD_GDEY075_WIDTH   800
#define EPD_GDEY075_HEIGHT  480
#define EPD_ROW_LEN         (EPD_GDEY075_WIDTH / 8)

#define BIT_SET(a,b)        ((a) |= (1U<<(b)))
#define BIT_CLR(a,b)        ((a) &= ~(1U<<(b)))

// color definitions for EPD
#define EPD_BLACK     0x00
#define EPD_WHITE     0xFF
#define EPD_RED       0xE0

#define T1      30  // charge balance pre-phase
#define T2      5   // optional extension
#define T3      30  // color change phase (b/w)
#define T4      5   // optional extension for one color

static const char * TAG =   EPD_GDEY075_MODEL;

static void epd_fill_screen(epd_panel_t *panel, uint8_t black, uint8_t color);
static esp_err_t epd_gdey075_del(epd_panel_t *panel);
static esp_err_t epd_gdey075_reset(epd_panel_t *panel);
static esp_err_t epd_gdey075_init(epd_panel_t *panel);
static esp_err_t epd_gdey075_get_info(epd_panel_t *panel, epd_panel_info_t * panel_info);
static esp_err_t epd_gdey075_set_flush_mode(epd_panel_t * panel, EPD_FLUSH_MODE mode);
static esp_err_t epd_gdey075_flush(epd_panel_t *panel, const lv_area_t * area, bool is_last, const void *color_data);
static esp_err_t epd_gdey075_rounder_cb(epd_panel_t *panel, lv_area_t *area);
static esp_err_t epd_gdey075_set_fb_cb(epd_panel_t *panel, uint8_t *buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
                           lv_color_t color, lv_opa_t opa);
static esp_err_t epd_gdey075_set_partial_area(epd_panel_io_handle_t io, int x1, int y1, int x2, int y2);
static esp_err_t epd_gdey075_refresh_full(epd_panel_t *panel);
static esp_err_t epd_gdey075_refresh_part(epd_panel_t *panel, int x1, int y1, int x2, int y2);
static esp_err_t epd_gdey075_power_on(epd_panel_t *panel);
static esp_err_t epd_gdey075_power_off(epd_panel_t *panel);
static esp_err_t epd_gdey075_hibernate(epd_panel_t *panel);


// experimental partial screen update LUTs with balanced charge
// LUTs are filled with zeroes

void epd_init_panel(epd_panel_t *panel);
void epd_init_partial_fast(epd_panel_t *panel);
void epd_fill_color(epd_panel_t *panel, lv_color_t color);

typedef struct {
    epd_panel_t base;
    epd_panel_io_handle_t io;
    int reset_gpio_num;
    int busy_gpio_num;
    EPD_FLUSH_MODE flush_mode;
    struct {
        uint8_t reset_level  : 1;
        uint8_t busy_level   : 1;
        uint8_t power_is_on  : 1;
        uint8_t hibernating  : 1;
        uint8_t partial_mode : 1;
    } prop;
} gdey075_panel_t;


typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t data_bytes; // Length of data in above data array; 0xFF = end of cmds.
} epd_init_cmd_t;


esp_err_t epd_new_panel_gdey075(const epd_panel_io_handle_t io, const epd_panel_dev_config_t *panel_dev_config, epd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    gdey075_panel_t * gdey075 = NULL;
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    gdey075 = calloc(1, sizeof(gdey075_panel_t));
    ESP_GOTO_ON_FALSE(gdey075, ESP_ERR_NO_MEM, err, TAG, "no mem for gdey075 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    if (panel_dev_config->busy_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->busy_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for BUSY line failed");
    }

    gdey075->io = io;
    gdey075->prop.power_is_on = false;
    gdey075->prop.hibernating = false;
    gdey075->reset_gpio_num = panel_dev_config->reset_gpio_num;
    gdey075->prop.reset_level = panel_dev_config->flags.reset_active_high != 0;
    gdey075->busy_gpio_num = panel_dev_config->busy_gpio_num;
    gdey075->prop.busy_level = panel_dev_config->flags.busy_active_high != 0;
    gdey075->base.del = epd_gdey075_del;
    gdey075->base.reset = epd_gdey075_reset;
    gdey075->base.init = epd_gdey075_init;
    gdey075->base.get_info = epd_gdey075_get_info;
    gdey075->base.set_flush_mode = epd_gdey075_set_flush_mode;
    gdey075->base.flush = epd_gdey075_flush;
    gdey075->base.rounder = epd_gdey075_rounder_cb;
    gdey075->base.set_px = epd_gdey075_set_fb_cb;
    gdey075->base.power_on = epd_gdey075_power_on;
    gdey075->base.light_sleep = epd_gdey075_power_off;
    gdey075->base.deep_sleep = epd_gdey075_hibernate;
    *ret_panel = &(gdey075->base);
    ESP_LOGD(TAG, "new gdey075 panel @%p", gdey075);

    return ESP_OK;

err:
    if (gdey075)
    {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(gdey075);
    }
    return ret;
}

static void epd_wait_until_idle(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);

    if (gdey075->busy_gpio_num >= 0)
    {
        // ESP_LOGI(TAG, "panel busy");
        unsigned char busy;
        do {
            vTaskDelay(pdMS_TO_TICKS(10));
            epd_panel_io_tx_param(gdey075->io, 0x71, NULL, 0);
            busy = gpio_get_level(gdey075->busy_gpio_num);
            busy = gdey075->prop.busy_level ? busy : !busy;
        } while(busy);
        // ESP_LOGI(TAG, "panel idle");
    }
}

static esp_err_t epd_gdey075_del(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);

    if (gdey075->reset_gpio_num >= 0)
    {
        gpio_reset_pin(gdey075->reset_gpio_num);
    }
    ESP_LOGD(TAG, "del gdey075 panel @%p", gdey075);
    free(gdey075);
    return ESP_OK;
}

static esp_err_t epd_gdey075_reset(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);

    // perform hardware reset
    if (gdey075->reset_gpio_num >= 0)
    {
        ESP_LOGI(TAG, "gdey075 panel reset");

        // gpio_set_level(gdey075->reset_gpio_num, !gdey075->prop.reset_level);
        // vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(gdey075->reset_gpio_num, gdey075->prop.reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(gdey075->reset_gpio_num, !gdey075->prop.reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));

        gdey075->prop.power_is_on = false;
        gdey075->prop.hibernating = false;

        return ESP_OK;
    }

    return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t epd_gdey075_init(epd_panel_t *panel)
{
    ESP_LOGI(TAG, "gdey075 panel init");

    // epd_init_panel(panel);

    // epd_fill_color(panel, lv_color_white());

    return ESP_OK;
}

/******************************************************************************
function :	fill screen
parameter:
******************************************************************************/
void epd_fill_color(epd_panel_t *panel, lv_color_t color)
{
    uint8_t full = lv_color_to8(color);
    ESP_LOGI(TAG, "epd_fill_color: 0x%X", full);
    switch (full)
    {
        case EPD_WHITE:
            epd_fill_screen(panel, EPD_WHITE, 0);
            break;
        case EPD_BLACK:
            epd_fill_screen(panel, EPD_BLACK, 0);
            break;
        case EPD_RED:
            epd_fill_screen(panel, 0, 0xFF);
            break;
        default:
            ESP_LOGE(TAG, "color 0x%X not support", full);
            break;
    }
}

static void epd_fill_screen(epd_panel_t *panel, uint8_t black, uint8_t color)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    size_t width  = EPD_GDEY075_WIDTH / 8;
    size_t height = EPD_GDEY075_HEIGHT;

    epd_panel_io_tx_param(io, 0x10, NULL, 0);
    for (size_t i=0; i < width * height; i++)
    {
        epd_panel_io_tx_param(io, -1, (uint8_t[]) { black }, 1);
    }

    epd_panel_io_tx_param(io, 0x13, NULL, 0);
    for (size_t i=0; i< width * height; i++)
    {
        epd_panel_io_tx_param(io, -1, (uint8_t[]) { color }, 1);
    }

    // epd_gdey075_refresh_full(panel);
}

static esp_err_t epd_gdey075_get_info(struct epd_panel_t *panel, epd_panel_info_t * panel_info)
{
    if (panel_info)
    {
        panel_info->width = EPD_GDEY075_WIDTH;
        panel_info->height = EPD_GDEY075_HEIGHT;
        panel_info->has_color = true;
        panel_info->model = EPD_GDEY075_MODEL;
    }
    return ESP_OK;
}

static esp_err_t epd_gdey075_set_flush_mode(epd_panel_t * panel, EPD_FLUSH_MODE mode)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    gdey075->flush_mode = mode;
    ESP_LOGI(TAG, "epd_gdey075_set_flush_mode %d", mode);
    return ESP_OK;
}

static esp_err_t epd_gdey075_flush(epd_panel_t *panel, const lv_area_t * area, bool is_last, const void *color_data)
{
    // ESP_LOGI(TAG, "flush%s, x1: %d, y1: %d, x2: %d, y2: %d", is_last ? " last" : "", area->x1, area->y1, area->x2 + 1, area->y2 + 1);
    // assert((x + width <= EPD_GDEY075_WIDTH) && (y + height <= EPD_GDEY075_HEIGHT) && "oversized");
    // assert((ind_area < sizeof(flush_area) / sizeof(lv_area_t)) && "oversized");
	static lv_area_t dirty_zone = { -1, -1, 0, 0 };

    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    /*if (gdey075->flush_mode == FLUSH_PARTIAL_FAST)
    {
        epd_init_partial_fast(panel);
    }
    else*/ if (!gdey075->prop.power_is_on)
    {
        epd_init_panel(panel);
    }

    uint8_t * data = (uint8_t *)color_data;
    int wb = (area->x2 - area->x1 + 1) >> 3;
    int h = area->y2 - area->y1 + 1;
    int k = 0;

    epd_panel_io_tx_param(io, 0x91, NULL, 0);  // partial in
    epd_gdey075_set_partial_area(io, area->x1, area->y1, area->x2, area->y2);

    if (gdey075->flush_mode != FLUSH_PARTIAL_FAST)
    {
        k = wb;

        epd_panel_io_tx_param(io, 0x10, NULL, 0);
        for (size_t i=0; i < h; i++)
        {
            epd_panel_io_tx_param(io, -1, data + wb * i * 2, wb);
        }
    }

    epd_panel_io_tx_param(io, 0x13, NULL, 0);
    for (size_t i=0; i < h; i++)
    {
        epd_panel_io_tx_param(io, -1, data + wb * i * 2 + k, wb);
    }

    epd_panel_io_tx_param(io, 0x92, NULL, 0);  // partial out

    if (dirty_zone.x1 < 0)
    {
        memcpy(&dirty_zone, area, sizeof(lv_area_t));
    }
    else if ((area->x1 != dirty_zone.x1 || area->y1 != dirty_zone.y2) && gdey075->flush_mode == FLUSH_PARTIAL_FAST)
    {
        ESP_LOGI(TAG, "gdey075 refresh the previous part");
        epd_gdey075_refresh_part(panel, dirty_zone.x1, dirty_zone.y1, dirty_zone.x2, dirty_zone.y2 - 1);
        memcpy(&dirty_zone, area, sizeof(lv_area_t));
    }
    dirty_zone.y2 = area->y1 + h;

    if (is_last)
    {
        if (gdey075->flush_mode == FLUSH_FULL)
        {
            epd_gdey075_refresh_full(panel);
        }
        else
        {
            epd_gdey075_refresh_part(panel, dirty_zone.x1, dirty_zone.y1, dirty_zone.x2, dirty_zone.y2 - 1);
        }
        ESP_LOGI(TAG, "gdey075 panel draw_bitmap done");

        dirty_zone.x1 = -1;

        epd_gdey075_power_off(panel);
    }

    return ESP_OK;
}

static esp_err_t epd_gdey075_set_fb_cb(epd_panel_t *panel, uint8_t *buf, lv_coord_t buf_w,
                        lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    uint16_t row_w = buf_w >> 3;
    uint16_t byte_index = (x >> 3) + row_w * y * 2;
    uint8_t bit_index = 7 - (x & 7);

    if (LV_COLOR_GET_R(color) < 5 && LV_COLOR_GET_G(color) < 5 && LV_COLOR_GET_B(color) < 5)    // black
    {
        BIT_CLR(buf[byte_index], bit_index);
        BIT_CLR(buf[byte_index + row_w], bit_index);
    }
    else if (LV_COLOR_GET_R(color) > LV_COLOR_GET_B(color) && LV_COLOR_GET_R(color) > LV_COLOR_GET_G(color)) // red
    {
        // ESP_LOGI(TAG, "RGB color: %d, %d, %d", LV_COLOR_GET_R(color), LV_COLOR_GET_G(color), LV_COLOR_GET_B(color));
        BIT_CLR(buf[byte_index], bit_index);
        BIT_SET(buf[byte_index + row_w], bit_index);
    }
    else        // white
    {
        BIT_SET(buf[byte_index], bit_index);
        BIT_CLR(buf[byte_index + row_w], bit_index);
    }

    return ESP_OK;
}

static esp_err_t epd_gdey075_rounder_cb(epd_panel_t *panel, lv_area_t *area)
{
    area->x1 = area->x1 & ~(0x7);
    area->x2 = area->x2 |  (0x7);

    return ESP_OK;
}

static esp_err_t epd_gdey075_set_partial_area(epd_panel_io_handle_t io, int x1, int y1, int x2, int y2)
{
    uint8_t data[9];
    data[0] = (x1 / 256);
    data[1] = (x1 % 256);
    data[2] = (x2 / 256);
    data[3] = (x2 % 256);
    data[4] = (y1 / 256);
    data[5] = (y1 % 256);
    data[6] = (y2 / 256);
    data[7] = (y2 % 256);
    data[8] = 1;
    return epd_panel_io_tx_param(io, 0x90, data, sizeof(data));  // partial window
}

static esp_err_t epd_gdey075_refresh_full(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    ESP_LOGI(TAG, "epd_gdey075_refresh_full enter");

    if (gdey075->prop.partial_mode)
    {
        epd_init_panel(panel);
    }

    epd_panel_io_tx_param(io, 0x12, NULL, 0);       // DISPLAY REFRESH
    epd_wait_until_idle(panel);

    return ESP_OK;
}

static esp_err_t epd_gdey075_refresh_part(epd_panel_t *panel, int x1, int y1, int x2, int y2)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    // ESP_LOGI(TAG, "epd_gdey075_refresh %d parts", cnt);
    ESP_LOGI(TAG, "epd_gdey075_refresh part: [%d,%d],[%d,%d]", x1, y1, x2+1, y2+1);

    if (gdey075->flush_mode == FLUSH_PARTIAL_FAST)
    {
        epd_init_partial_fast(panel);
    }
    else if (!gdey075->prop.power_is_on)
    {
        epd_init_panel(panel);
    }

    epd_panel_io_tx_param(io, 0x91, NULL, 0);       // partial in

    epd_gdey075_set_partial_area(io, x1, y1, x2, y2);

    epd_panel_io_tx_param(io, 0x12, NULL, 0);       // DISPLAY REFRESH
    epd_wait_until_idle(panel);

    epd_panel_io_tx_param(io, 0x92, NULL, 0);       // partial out

    return ESP_OK;
}

static esp_err_t epd_gdey075_power_on(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    if (!gdey075->prop.power_is_on)
    {
        ESP_LOGI(TAG, "epd_gdey075_power_on enter");
        epd_panel_io_tx_param(io, 0x04, NULL, 0);                   //power on
        epd_wait_until_idle(panel);

        gdey075->prop.power_is_on = true;
    }

    return ESP_OK;
}

// light sleep
static esp_err_t epd_gdey075_power_off(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    ESP_LOGI(TAG, "epd_gdey075_power_off enter");

    epd_panel_io_tx_param(io, 0x02, NULL, 0);                   //power off
    epd_wait_until_idle(panel);

    gdey075->prop.power_is_on = false;

    return ESP_OK;
}

// deep sleep
static esp_err_t epd_gdey075_hibernate(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    ESP_LOGI(TAG, "epd_gdey075_hibernate enter");

    if (gdey075->prop.power_is_on)
    {
        epd_gdey075_power_off(panel);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    epd_panel_io_tx_param(io, 0x07, (uint8_t[]) { 0xA5 }, 1);   //deep sleep
    gdey075->prop.hibernating = true;

    return ESP_OK;
}

void epd_init_panel(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    ESP_LOGI(TAG, "epd_init_panel enter");

    if (gdey075->prop.hibernating) epd_gdey075_reset(panel);

    // reference to https://github.com/GoodDisplay/E-paper-Display-Library-of-GoodDisplay/blob/main/Tri-Color_E-paper-Display/7.5inch_UC8179_GDEY075Z08_800x480/ESP32-Arduino%20IDE/GDEY075Z08_Arduino.ino
    const epd_init_cmd_t vendor_init[] = {
        {0x01, {0x07, 0x07, 0x3f, 0x3f}, 4},    // POWER SETTING,VGH=20V,VGL=-20V,VDH=15V,VDL=-15V
        {0x04, {}, 0},                          // POWER ON
        {0x00, {0x0F}, 1},                      // PANNEL SETTING, LUT from OTP, KWR mode
        {0x61, {0x03, 0x20, 0x01, 0xE0}, 4},    // source 800, gate 480
        {0x15, {0x00}, 1},                      // single SPI mode
        {0x50, {0x11, 0x07}, 2},                // VCOM AND DATA INTERVAL SETTING
        {0x60, {0x22}, 1},                      // TCON SETTING
    };

    for (int i = 0; i < sizeof(vendor_init)/sizeof(epd_init_cmd_t); ++i)
    {
        epd_panel_io_tx_param(io, vendor_init[i].cmd, vendor_init[i].data, vendor_init[i].data_bytes & 0x1F);
        if (vendor_init[i].cmd == 0x04)
        {
            epd_wait_until_idle(panel);
            gdey075->prop.power_is_on = true;
        }
    }

    gdey075->prop.partial_mode = false;
}

void epd_init_partial_fast(epd_panel_t *panel)
{
    gdey075_panel_t *gdey075 = __containerof(panel, gdey075_panel_t, base);
    epd_panel_io_handle_t io = gdey075->io;

    ESP_LOGI(TAG, "epd_init_partial enter");

    if (gdey075->prop.hibernating) epd_gdey075_reset(panel);

    const epd_init_cmd_t vendor_init[] = {
        {0x01, {0x07, 0x07, 0x3f, 0x3f}, 4},    // POWER SETTING,VGH=20V,VGL=-20V,VDH=15V,VDL=-15V
        {0x04, {}, 0},                          // POWER ON
        {0x00, {0x3f}, 1},                      // PANNEL SETTING, LUT from register
        {0x61, {0x03, 0x20, 0x01, 0xE0}, 4},    // source 800, gate 480
        {0x15, {0x00}, 1},                      // single SPI mode
        {0x60, {0x22}, 1},                      // TCON SETTING
        {0x82, {0x30}, 1},                      // vcom_DC setting, -2.5V same value as in OTP
        {0x50, {0x39, 0x07}, 2},                // VCOM AND DATA INTERVAL SETTING, LUTKW, N2OCP: copy new to old
    };
    for (int i = 0; i < sizeof(vendor_init)/sizeof(epd_init_cmd_t); ++i)
    {
        epd_panel_io_tx_param(io, vendor_init[i].cmd, vendor_init[i].data, vendor_init[i].data_bytes & 0x1F);
        if (vendor_init[i].cmd == 0x04)
        {
            epd_wait_until_idle(panel);
            gdey075->prop.power_is_on = true;
        }
    }

    char lut[42] = { 0, T1, T2, T3, T4, 1, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    epd_panel_io_tx_param(io, 0x20, lut, sizeof(lut));  // LUTC
    epd_panel_io_tx_param(io, 0x21, lut, sizeof(lut));  // LUTWW

    lut[0] = 0x48;
    epd_panel_io_tx_param(io, 0x22, lut, sizeof(lut));  // LUTKW

    lut[0] = 0x84;
    epd_panel_io_tx_param(io, 0x23, lut, sizeof(lut));  // LUTWK

    lut[0] = 0;
    epd_panel_io_tx_param(io, 0x24, lut, sizeof(lut));  // LUTKK
    epd_panel_io_tx_param(io, 0x25, lut, sizeof(lut));  // LUTBD

    gdey075->prop.partial_mode = true;
}
