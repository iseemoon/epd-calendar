#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_littlefs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/rtc_io.h"
#include "misc/app_nvs.h"
#include "misc/app_key.h"
#include "common/common.h"
#include "cloud/app_cloud.h"
#include "ui_lvgl/ui_epd.h"


#define UART_PORT_NUM           
#define UART_BAUD_RATE          460800

static const char *TAG = "==>";

static HUMITURE humiture = { 0 };
static BATTERY battery = { 0 };

static void prepare()
{
    esp_err_t ret;
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;
    uint32_t size_flash_chip = 0;

    // ESP_LOGI(TAG, "sizeof(lv_color_t) = %d", sizeof(lv_color_t));
    // ESP_LOGI(TAG, "sizeof(int) = %d", sizeof(int));
    // ESP_LOGI(TAG, "sizeof(long) = %d", sizeof(long));
    // ESP_LOGI(TAG, "sizeof(float) = %d", sizeof(float));
    // ESP_LOGI(TAG, "sizeof(short) = %d", sizeof(short));
    // ESP_LOGI(TAG, "LV_COLOR_DEPTH = %d", LV_COLOR_DEPTH);
    printf("\n\n\n");
    printf("==============================================================\n");
    esp_flash_get_size(NULL, &size_flash_chip);
    printf("[===                   Flash size: %uMB                    ===]\n", (unsigned int)size_flash_chip >> 20);

    setenv("TZ", "CST-8", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    printf("[===               %s               ===]\n", strftime_buf);
    printf("==============================================================\n");

    ESP_LOGI(TAG, "Wakeup cause: %d", esp_sleep_get_wakeup_cause());

    //Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initializing LittleFS
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/fs",
        .partition_label = "fs",
        .format_if_mount_failed = true,
        .dont_mount = false,
    };
    ret = esp_vfs_littlefs_register(&conf);
    switch (ret)
    {
    case ESP_OK:
        ESP_LOGI(TAG, "LittleFS initialize done");
        break;
    case ESP_FAIL:
        ESP_LOGE(TAG, "Failed to mount or format filesystem");
        return;
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE(TAG, "Failed to find LittleFS partition");
        return;
    default:
        ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
        struct stat st;
        if (stat("/fs/fonts/songti.ttf", &st) == 0)
        {
            ESP_LOGI(TAG, "Font file size: %ld", st.st_size);
        }
        else
        {
            ESP_LOGE(TAG, "Font file not found");
        }
    }
    else
    {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
    }

    // uart_config_t uart_config = {
    //     .baud_rate = UART_BAUD_RATE,
    //     .data_bits = UART_DATA_8_BITS,
    //     .parity    = UART_PARITY_DISABLE,
    //     .stop_bits = UART_STOP_BITS_1,
    //     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    //     .source_clk = UART_SCLK_DEFAULT,
    // };

    // ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    // ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    // ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));
}

static void enter_light_sleep()
{
    gpio_config_t config = {
        .pin_bit_mask = GPIO_KEY_MASK,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = false,
        .pull_up_en = false,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&config);                               // Initialize GPIO failed
    /* Enable wake up from GPIO */
    gpio_wakeup_enable(GPIO_KEY2, GPIO_INTR_LOW_LEVEL); // Enable gpio wakeup failed
    gpio_wakeup_enable(GPIO_KEY3, GPIO_INTR_LOW_LEVEL); // Enable gpio wakeup failed
    gpio_wakeup_enable(GPIO_KEY4, GPIO_INTR_LOW_LEVEL); // Enable gpio wakeup failed
    esp_sleep_enable_gpio_wakeup();                     // Configure gpio as wakeup source failed

    /* Enable wakeup from light sleep by timer */
    esp_sleep_enable_timer_wakeup(1000000LL * 60);      // Configure timer as wakeup source failed

    int64_t t_before_us = esp_timer_get_time();
    esp_light_sleep_start();        /* Enter sleep mode */
    /* Get timestamp after waking up from sleep */
    int64_t t_after_us = esp_timer_get_time();

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    ESP_LOGI(TAG, "Wakeup from light sleep, reason: %d, t=%lld ms, slept for %lld ms", cause, t_after_us / 1000, (t_after_us - t_before_us) / 1000);

    switch (cause)
    {
    case ESP_SLEEP_WAKEUP_GPIO:
        while (gpio_get_level(GPIO_KEY2) == GPIO_INTR_LOW_LEVEL
            || gpio_get_level(GPIO_KEY3) == GPIO_INTR_LOW_LEVEL
            || gpio_get_level(GPIO_KEY4) == GPIO_INTR_LOW_LEVEL) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
    default:
        break;
    }
}

/*
开机后每分钟测量一次温度，每十分钟测量一次电池电压
*/
static void measureTask(void *pvParameter)
{
    begin_sens_measure();

    time_t now = time(NULL);
    if (battery.meas_time == 0 || difftime(now, battery.meas_time) > 600)
    {
        measue_battery(&battery);
        ESP_LOGI(TAG, "Battery voltage: %d mV, %f%%", battery.voltage, battery.percent);
    }

    if (humiture.meas_time == 0 || difftime(now, humiture.meas_time) > 60)
    {
        measue_humiture(&humiture);
        ESP_LOGI(TAG, "Temperature: %f, humidity: %f", humiture.temperature, humiture.humidity);
    }

    end_sens_measure();

    vTaskDelete(NULL);
}

void app_main(void)
{
    prepare();                              // initialize time zone and file system
    init_epd_panel();
    init_lvgl();

    ESP_LOGI(TAG, "Initialization done, free heap: %u", (unsigned int)esp_get_free_heap_size());

    do {
        esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
        if (cause == ESP_SLEEP_WAKEUP_UNDEFINED)
        {
            nvs_set_screen(0);
            cloud_sync();
            xTaskCreate(measureTask, "measure", 4096, NULL, tskIDLE_PRIORITY, NULL);
        }
        else if (cause == ESP_SLEEP_WAKEUP_EXT0 || sync_elapsed_time() > 3600) // 2: RTC_IO, wakeup by GPIO
        {
            cloud_sync();
        }
        else
        {
            xTaskCreate(measureTask, "measure", 4096, NULL, tskIDLE_PRIORITY, NULL);
        }
    } while (ui_loop());

    // begin Wake up in deep sleep
    // Configure pullup/downs via RTCIO to tie wakeup pins to inactive level during deepsleep.
    // EXT0 resides in the same power domain (RTC_PERIPH) as the RTC IO pullup/downs.
    // No need to keep that power domain explicitly, unlike EXT1.
    // ESP_LOGI(TAG, "Enabling EXT0 wakeup on GPIO%d", GPIO_KEY3);
    esp_sleep_enable_ext0_wakeup(GPIO_KEY3, 0);     // low level to wakeup
    rtc_gpio_pullup_en(GPIO_KEY3);
    rtc_gpio_pulldown_dis(GPIO_KEY3);

    do {
        ESP_LOGI(TAG, "Entering deep sleep...");
        time_t sleep_time = time(NULL);
        esp_deep_sleep(1000000LL * (60 - sleep_time % 60));     // esp_deep_sleep_start();
    } while (0);
    // end Wake up in deep sleep
}
