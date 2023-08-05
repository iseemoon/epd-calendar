#include "common.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "sht30.h"
#include "misc/app_bat.h"

#define TAG                     "COMMON"

#define GPIO_MEASUE_EN          GPIO_NUM_25
#define GPIO_BAT_ADC            GPIO_NUM_36


void begin_sens_measure()
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_MEASUE_EN
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_MEASUE_EN, 1);
    msleep(100);
}

void end_sens_measure()
{
    gpio_set_level(GPIO_MEASUE_EN, 0);
}

_Bool measue_humiture(HUMITURE * humiture)
{
    uint8_t temp;
    uint8_t humi;
    if (sht30_init())
    {
        msleep(100);
        if (sht30_get_value(&temp, &humi))
        {
            humiture->temperature = temp;
            humiture->humidity = humi;
            humiture->meas_time = time(NULL);
            return true;
        }
    }

    return false;
}

void measue_battery(BATTERY * battery)
{
    battery->voltage = get_battery_vol();
    battery->percent = get_battery_percent(battery->voltage);
    battery->meas_time = time(NULL);
}
