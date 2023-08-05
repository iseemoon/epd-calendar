#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


#define TAG                 "ADC"
#define ADC1_BAT_CHN        ADC_CHANNEL_0
#define BAT_DET_TIMES       3


// voltage to raw data struct
typedef struct {
    uint32_t vol;
    uint16_t raw;
    uint16_t per;   // percent
} vol_to_raw_t;

/* Set thresholds, approx. 2.00V - 3.30V */
static const vol_to_raw_t g_v0l_to_raw[] = {
    {2800, 1000, 0},
    {2900, 1200, 10},
    {3000, 1300, 10},
    {3100, 1400, 10},
    {3200, 1500, 10},
    {3300, 1600, 20},
    {3400, 1700, 30},
    {3500, 1800, 40},
    {3600, 1900, 50},
    {3700, 2090, 60},
    {3800, 2190, 70},
    {4000, 2290, 80},
    {4100, 2390, 90},
    {4200, 2490, 100},
};

static const int VAL_TO_RAW_NUM = sizeof(g_v0l_to_raw) / sizeof(vol_to_raw_t);

float get_battery_percent(int voltage)
{
    float per = 0;
    for (int i = 0; i < VAL_TO_RAW_NUM - 1; i++)
    {
        if (g_v0l_to_raw[i].vol <= voltage && g_v0l_to_raw[i + 1].vol >= voltage)
        {
            uint32_t vol_dif = g_v0l_to_raw[i + 1].vol - g_v0l_to_raw[i].vol;
            uint16_t per_dif = g_v0l_to_raw[i + 1].per - g_v0l_to_raw[i].per;
            per = g_v0l_to_raw[i].per + (voltage - g_v0l_to_raw[i].vol) * per_dif / vol_dif;
            break;
        }
    }
    return per;
}

static uint32_t adc_raw_to_vol(uint32_t adc_raw)
{
    if (adc_raw < g_v0l_to_raw[0].raw)
    {
        return 0;
    }
    else if (adc_raw >= g_v0l_to_raw[VAL_TO_RAW_NUM - 1].raw)
    {
        return g_v0l_to_raw[VAL_TO_RAW_NUM - 1].vol;
    }

    uint32_t vol = 0;
    for (int i = 0; i < VAL_TO_RAW_NUM - 1; i++)
    {
        if (g_v0l_to_raw[i].raw <= adc_raw && g_v0l_to_raw[i + 1].raw >= adc_raw)
        {
            uint16_t raw_dif = g_v0l_to_raw[i + 1].raw - g_v0l_to_raw[i].raw;
            uint32_t vol_dif = g_v0l_to_raw[i + 1].vol - g_v0l_to_raw[i].vol;
            vol = g_v0l_to_raw[i].vol + (adc_raw - g_v0l_to_raw[i].raw) * vol_dif / raw_dif;
            break;
        }
    }
    return vol;
}

int get_battery_vol()
{
    int adc_raw;
    int voltage;
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_BAT_CHN, &config));

    int sum = 0;
    for (int i=0; i<BAT_DET_TIMES; ++i)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC1_BAT_CHN, &adc_raw));
        sum += adc_raw;
        // ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_BAT_CHN, adc_raw);
    }
    // ESP_LOGI(TAG, "Battery detection take %lld ms", (esp_timer_get_time() - start) / 1000);
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));

    adc_raw = sum / BAT_DET_TIMES;
    uint32_t vol = adc_raw_to_vol(adc_raw);
    ESP_LOGI(TAG, "ADC channel-%d raw data: %d, bat vol: %lu", ADC1_BAT_CHN, adc_raw, vol);

    return (int)vol;
}
