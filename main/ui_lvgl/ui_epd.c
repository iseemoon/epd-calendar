#include "ui_epd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "epd_panel_io.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "driver/spi_common.h"
#include "misc/app_nvs.h"
#include "misc/app_key.h"
#include "common/common.h"


#define EPD_HOST                SPI2_HOST
#define EEPD_CLOCK_HZ           (20 * 1000 * 1000)
#define PIN_NUM_MOSI            23
#define PIN_NUM_SCLK            18
#define PIN_NUM_EPD_DC          19
#define PIN_NUM_EPD_CS          5
#define PIN_NUM_EPD_BUSY        17
#define PIN_NUM_EPD_RST         16

// #if GDEY075Z08
#define EPD_H_RES               800
#define EPD_V_RES               480
// #endif

#define MSG_ID_EXIT             0
#define MSG_ID_SHOW_SCREEN      1
#define MSG_ID_WIFI_SET         2

#define LVGL_TICK_PERIOD_MS     2

#define SCREEN_NUM              6

#define WORK_QUEUE_LEN          32
#define BIT_LVGL_REFRESH         0x01
#define BIT_LVGL_PENDING         0x02
#define BIT_LVGL_EXIT            0x04


extern lv_font_t chinese_hei_16;

extern void lv_port_indev_init(void);
extern void init_home_ctx(struct _SCR_CONTEXT * ctx);
extern void init_wifi_ctx(struct _SCR_CONTEXT * ctx);
extern void init_calendar_ctx(struct _SCR_CONTEXT * ctx);
extern void init_digi_clock_ctx(struct _SCR_CONTEXT * ctx);
extern void init_analogue_clock_ctx(struct _SCR_CONTEXT * ctx);


static RTC_DATA_ATTR uint8_t sleep_scr_id;

static epd_panel_handle_t panel_handle = NULL;
static SCR_CONTEXT *       act_scr_ctx = NULL;

static lv_disp_drv_t        disp_drv;   // contains callback functions
static lv_disp_draw_buf_t   disp_buf;   // contains internal graphic buffer(s) called draw buffer(s)
static lv_disp_t *          disp;

static const char * TAG = "UI";


typedef struct
{
    uint32_t id;
    uint32_t val;
    void *   payload;
    uint32_t length;
    // uint32_t seconds;
    // uint32_t microseconds;
} msg_info_t;

static QueueHandle_t work_queue;
static TaskHandle_t task_lvgl = NULL;

static void on_key_event(gpio_num_t gpio, KEY_EVENT event);

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    epd_panel_handle_t handle = (epd_panel_handle_t) drv->user_data;

	bool is_last = lv_disp_flush_is_last(drv);
    
	epd_panel_flush(handle, area, is_last, color_map);
	lv_disp_flush_ready(drv);
	
    if (is_last)
    {
        xTaskNotify(task_lvgl, BIT_LVGL_PENDING, eSetBits);
    }
}

static void lvgl_rounder_cb(struct _lv_disp_drv_t * drv, lv_area_t * area)
{
    epd_panel_handle_t handle = (epd_panel_handle_t) drv->user_data;

    epd_rounder(handle, area);
}

static void lvgl_set_px_cb(struct _lv_disp_drv_t * drv, uint8_t * buf, lv_coord_t buf_w,
            lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    epd_panel_handle_t handle = (epd_panel_handle_t) drv->user_data;

    epd_set_px(handle, buf, buf_w, x, y, color, opa);
}

static void lvgl_increase_tick(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void set_flush_mode(EPD_FLUSH_MODE mode)
{
    epd_set_flush_mode(panel_handle, mode);
}

void init_epd_panel()
{
    // Initialize SPI bus
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_SCLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EPD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(EPD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Install panel IO
    epd_panel_io_handle_t io_handle = NULL;
    epd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_EPD_DC,
        .cs_gpio_num = PIN_NUM_EPD_CS,
        .pclk_hz = EEPD_CLOCK_HZ,
        .epd_cmd_bits = 8,
        .epd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = &disp_drv,
        .flags = {
            .dc_low_on_data = 0,    /*!< GDEY075, 0: command, 1: data */
            .sio_mode = 1,          /*!< Read and write through a single data line (MOSI) */
            .lsb_first = 0,         /*!< transmit LSB bit first if lsb_first was set to 1*/
            .cs_high_active = 0,    /*!< CS line is high active if cs_high_active was set to 1*/
        }
    };
    // Attach the EPD to the SPI bus, need call epd_panel_io_del(io_handle) to release resource
    ESP_ERROR_CHECK(epd_new_panel_io_spi(EPD_HOST, &io_config, &io_handle));

    // Install epd panel driver
    epd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_EPD_RST,
        .busy_gpio_num = PIN_NUM_EPD_BUSY,
        .bits_per_pixel = 8,
        .flags = {
            .reset_active_high = 0,     // reset signal, active low
            .busy_active_high = 0,      // low means the chip is busy
        }
    };
    ESP_ERROR_CHECK(epd_new_panel(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(epd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(epd_panel_init(panel_handle));

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    // epd_panel_delete(panel_handle);
}

void init_lvgl()
{
    ESP_LOGI(TAG, "==> LVGL initial start");
    // Initialize LVGL library
    lv_init();

    // initialize LVGL draw buffers
    lv_color_t * buf1 = heap_caps_malloc(EPD_H_RES * 10 / 4, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf1);
    // lv_color_t * buf2 = heap_caps_malloc(EPD_H_RES * 10 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    // assert(buf2);
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, EPD_H_RES * 10);

    // Register display driver to LVGL
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EPD_H_RES;
    disp_drv.ver_res = EPD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.rounder_cb = lvgl_rounder_cb;
    disp_drv.set_px_cb = lvgl_set_px_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    disp = lv_disp_drv_register(&disp_drv);

    // Install LVGL tick timer
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_increase_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 2 * 1000));

    user_key_init(on_key_event);
    lv_port_indev_init();

    ESP_LOGI(TAG, "==> LVGL initial done");
}

static void on_key_event(gpio_num_t gpio, KEY_EVENT event)
{
    if (act_scr_ctx)
    {
        if (event == KEY_EVT_DOWN && act_scr_ctx->on_key_down)
        {
            switch (gpio)
            {
            case GPIO_NUM_34:
                act_scr_ctx->on_key_down(act_scr_ctx, LV_KEY_ENTER);
                break;
            case GPIO_NUM_35:
                act_scr_ctx->on_key_down(act_scr_ctx, LV_KEY_LEFT);
                break;
            case GPIO_NUM_39:
                act_scr_ctx->on_key_down(act_scr_ctx, LV_KEY_RIGHT);
                break;
            default:
                break;
            }
        }
        else if (event == KEY_EVT_UP && act_scr_ctx->on_key_up)
        {
            switch (gpio)
            {
            case GPIO_NUM_34:
                act_scr_ctx->on_key_up(act_scr_ctx, LV_KEY_ENTER);
                break;
            case GPIO_NUM_35:
                act_scr_ctx->on_key_up(act_scr_ctx, LV_KEY_LEFT);
                break;
            case GPIO_NUM_39:
                act_scr_ctx->on_key_up(act_scr_ctx, LV_KEY_RIGHT);
                break;
            default:
                break;
            }
        }
    }
}

static SCR_CONTEXT * get_screen_ctx_by_id(uint8_t id)
{
    static SCR_CONTEXT * epd_screens[SCREEN_NUM] = { 0 };

    SCR_CONTEXT * ctx = NULL;
    if (id < SCREEN_NUM)
    {
        if (epd_screens[id] == NULL)
        {
            switch (id)
            {
            case SCR_ID_HOME:
                epd_screens[id] = (SCR_CONTEXT *)malloc(sizeof(SCR_CONTEXT));
                init_home_ctx(epd_screens[id]);
                break;
            case SCR_ID_WIFI:
                epd_screens[id] = (SCR_CONTEXT *)malloc(sizeof(SCR_CONTEXT));
                init_wifi_ctx(epd_screens[id]);
                break;
            case SCR_ID_DIGI_CLK:
                epd_screens[id] = (SCR_CONTEXT *)malloc(sizeof(SCR_CONTEXT));
                init_digi_clock_ctx(epd_screens[id]);
                break;
            case SCR_ID_ANALOGUE_CLK:
                epd_screens[id] = (SCR_CONTEXT *)malloc(sizeof(SCR_CONTEXT));
                init_analogue_clock_ctx(epd_screens[id]);
                break;
            case SCR_ID_CALENDAR:
                epd_screens[id] = (SCR_CONTEXT *)malloc(sizeof(SCR_CONTEXT));
                init_calendar_ctx(epd_screens[id]);
                break;
            default:
                break;
            }
        }

        ctx = epd_screens[id];
        if (ctx && ctx->scr == NULL)
        {
            ctx->create_ui(ctx);
        }
    }

    return ctx;
}

void show_screen_by_id(uint8_t id)
{
    if (work_queue)
    {
        ESP_LOGI(TAG, "notify show screen %u", id);
        msg_info_t msg = { 0 };
        msg.id = MSG_ID_SHOW_SCREEN;
        msg.val = id;
        xQueueSend(work_queue, &msg, portMAX_DELAY);
    }
}

void notify_wifi_set()
{
    if (work_queue)
    {
        msg_info_t msg = { 0 };
        msg.id = MSG_ID_WIFI_SET;
        xQueueSend(work_queue, &msg, portMAX_DELAY);
    }
}

void notify_exit()
{
    if (work_queue)
    {
        ESP_LOGI(TAG, "notify exit");
        msg_info_t msg = { 0 };
        msg.id = MSG_ID_EXIT;
        xQueueSend(work_queue, &msg, portMAX_DELAY);
    }
}

static void display_screen(SCR_CONTEXT * ctx)
{
    // {
    //     ESP_LOGE(TAG, "create screen %u error", id);
    //     show_msg_scr("网络未联接, 将于5分钟后重试...");
    //     msleep(30000);
    //     esp_deep_sleep(1000000LL * 270);
    // }
}

static void lgvl_task(void *pvParameter)
{
    uint32_t ev = 0;
    TickType_t wait_time = portMAX_DELAY;
    ESP_LOGI(TAG, "lvgl task %p running...", xTaskGetCurrentTaskHandle());
    while (1)
    {
        if (xTaskNotifyWait(0, BIT_LVGL_REFRESH | BIT_LVGL_PENDING, &ev, wait_time) == pdFALSE)
        {
            lv_timer_handler();
        }
        else if (BIT_LVGL_REFRESH & ev)
        {
            ESP_LOGI(TAG, "lvgl refresh begin");
            wait_time = pdMS_TO_TICKS(10);
        }
        else if (BIT_LVGL_PENDING & ev)
        {
            ESP_LOGI(TAG, "lvgl refresh pause");
            wait_time = portMAX_DELAY;
            if (act_scr_ctx && act_scr_ctx->draw_finished)
            {
                act_scr_ctx->draw_finished(act_scr_ctx);
            }
        }
        else if (BIT_LVGL_EXIT & ev)
        {
            break;
        }
    }

    ESP_LOGI(TAG, "lvgl task leave");
}

void show_msg_scr(const char * msg)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t * label = lv_label_create(scr);
    lv_label_set_text_static(label, msg);
    lv_obj_set_style_text_font(label, &chinese_hei_16, 0);
    lv_obj_set_style_text_color(label, lv_color_make(0, 0, 0xFF), 0);
    lv_obj_center(label);

    lv_scr_load(scr);
    lv_refr_now(disp);
}

bool ui_loop()
{
    bool loop = true;
    msg_info_t msg = {
        .id = MSG_ID_SHOW_SCREEN,
        .val = !nvs_read_wifi_config(NULL) ? SCR_ID_WIFI : nvs_get_screen(),
        .payload = NULL,
        .length = 0
    };

    work_queue = xQueueCreate(WORK_QUEUE_LEN, sizeof(msg_info_t));
    xTaskCreate(lgvl_task, "lvgl", 4096, NULL, tskIDLE_PRIORITY, &task_lvgl);

    do {
        switch (msg.id)
        {
        case MSG_ID_SHOW_SCREEN:
            SCR_CONTEXT * ctx = get_screen_ctx_by_id(msg.val);
            if (ctx)
            {
                ESP_LOGI(TAG, "show screen %lu, prev: %p", msg.val, act_scr_ctx);

                xTaskNotify(task_lvgl, BIT_LVGL_PENDING, eSetBits);
                lv_scr_load(ctx->scr);
                ctx->prepare(ctx, sleep_scr_id);
                xTaskNotify(task_lvgl, BIT_LVGL_REFRESH, eSetBits);

                if (act_scr_ctx)
                {
                    act_scr_ctx->discard_ui(act_scr_ctx);
                }
                act_scr_ctx = ctx;
                sleep_scr_id = act_scr_ctx->scr_id;
            }
            else
            {
                ESP_LOGE(TAG, "failed to show screen %lu", msg.val);
            }
            break;
        case MSG_ID_WIFI_SET:
            ESP_LOGI(TAG, "WiFi configuration done");
            nvs_set_screen(0);
            vTaskDelete(task_lvgl);
            vQueueDelete(work_queue);
            return true;
        case MSG_ID_EXIT:
            loop = false;
            break;
        }
    } while (loop && xQueueReceive(work_queue, &msg, portMAX_DELAY) == pdTRUE);

    vTaskDelete(task_lvgl);
    vQueueDelete(work_queue);

    return false;
}