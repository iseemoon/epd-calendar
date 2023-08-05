/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "app_cloud.h"
#include "misc/app_wifi.h"
#include "misc/app_nvs.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#define CMA_WEATHER_URL     "https://weather.cma.cn/api/weather/view"
#define URL_GET_MYIP        "https://weather.cma.cn/api/weather/view"
#define MAX_WEATHER_RESP    4096

static const char *TAG = "CLOUD";

static RTC_DATA_ATTR time_t last_sync_time = 0;
static RTC_DATA_ATTR LOCATION location = { 0 };

static void initialize_sntp(sntp_sync_time_cb_t callback)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_time_sync_notification_cb(callback);

    if (SNTP_MAX_SERVERS > 0) sntp_setservername(0, "ntp.aliyun.com");
    if (SNTP_MAX_SERVERS > 1) sntp_setservername(1, "ntp.tencent.com");
    if (SNTP_MAX_SERVERS > 2) sntp_setservername(2, "cn.pool.ntp.org");
    if (SNTP_MAX_SERVERS > 3) sntp_setservername(3, "pool.ntp.org");
    if (SNTP_MAX_SERVERS > 4) sntp_setservername(4, "time.windows.com");

    sntp_init();
}

bool app_sntp_sync_time(sntp_sync_time_cb_t callback)
{
    int retry = 0;

    ESP_ERROR_CHECK( esp_netif_init() );

    initialize_sntp(callback);

    // wait for time to be set
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    if (sntp_enabled())
    {
        sntp_stop();
    }

    return retry < retry_count;
}

bool app_sync_locatioin(LOCATION * location)
{
    esp_err_t err = ESP_FAIL;
    esp_http_client_config_t config = {
        .url = CMA_WEATHER_URL,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
    {
        ESP_LOGE(TAG, "Fail to init HTTP client");
        return false;
    }

    if ((err = esp_http_client_open(client, 0)) != ESP_OK)
    {
        esp_http_client_cleanup(client);
        ESP_LOGE(TAG, "Fail to open HTTP connection: %s", esp_err_to_name(err));
        return false;
    }

    int content_length =  esp_http_client_fetch_headers(client);
    if (content_length >= 0 && content_length < MAX_WEATHER_RESP)
    {
        char * buffer = (char*)malloc(MAX_WEATHER_RESP);
        content_length = esp_http_client_read_response(client, buffer, MAX_WEATHER_RESP - 1);
        ESP_LOGI(TAG, "cma weather response length: %d", content_length);

        do {
            if (content_length <= 0) {
                ESP_LOGE(TAG, "weather http boby length error");
                break;
            }

            buffer[content_length] = '\0';

            cJSON *root = cJSON_Parse(buffer);
            if (!root) {
                ESP_LOGE(TAG, "Fail to parse weathre response: bad json format\n%s", buffer);
                break;
            }

            cJSON * jCode = cJSON_GetObjectItem(root, "code");
            if (!jCode || !cJSON_IsNumber(jCode) || cJSON_GetNumberValue(jCode) != 0) {
                ESP_LOGE(TAG, "resp code is NOT zero");
                break;
            }

            cJSON * jData = cJSON_GetObjectItem(root, "data");
            if (!jData || !cJSON_IsObject(jData)) {
                ESP_LOGE(TAG, "resp data NOT found");
                break;
            }

            cJSON * jLoc = cJSON_GetObjectItem(jData, "location");
            if (jLoc && cJSON_IsObject(jLoc))
            {
                // { "id":"59493", "name":"深圳", "path":"中国, 广东, 深圳", "longitude":114, "latitude":22.53, "timezone": 8 }
                cJSON * jObj = cJSON_GetObjectItem(jLoc, "name");
                if (jObj && cJSON_IsString(jObj))
                {
                    strncpy(location->city, cJSON_GetStringValue(jObj), sizeof(location->city) - 1);
                }

                jObj = cJSON_GetObjectItem(jLoc, "path");
                if (jObj && cJSON_IsString(jObj))
                {
                    strncpy(location->province, cJSON_GetStringValue(jObj), sizeof(location->province) - 1);
                }

                jObj = cJSON_GetObjectItem(jLoc, "latitude");
                if (jObj && cJSON_IsNumber(jObj))
                {
                    location->latitude = cJSON_GetNumberValue(jObj);
                }

                jObj = cJSON_GetObjectItem(jLoc, "longitude");
                if (jObj && cJSON_IsNumber(jObj))
                {
                    location->longitude = cJSON_GetNumberValue(jObj);
                }

                jObj = cJSON_GetObjectItem(jLoc, "timezone");
                if (jObj && cJSON_IsNumber(jObj))
                {
                    location->timezone = cJSON_GetNumberValue(jObj);
                }

                err = ESP_OK;
            }

            cJSON * jDaily = cJSON_GetObjectItem(jData, "daily");
            if (jDaily && cJSON_IsArray(jDaily))
            {
                int sz = cJSON_GetArraySize(jDaily);
                if (sz > 0)
                {
                    WEATHER * pWeather = (WEATHER*)malloc(sizeof(WEATHER));
                    nvs_erase_space(NV_NAMESPACE_WEATHER);
                    for (int i=0; i<sz; ++i)
                    {
                        cJSON * jWeath = cJSON_GetArrayItem(jDaily, i);
                        memset(pWeather, 0, sizeof(WEATHER));
                        cJSON * jObj = cJSON_GetObjectItem(jWeath, "date");
                        if (jObj && cJSON_IsString(jObj))
                        {
                            int year, month, day;
                            sscanf(cJSON_GetStringValue(jObj), "%d/%d/%d", &year, &month, &day);
                            pWeather->year = year;
                            pWeather->month = month;
                            pWeather->day = day;
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "high");
                        if (jObj && cJSON_IsNumber(jObj))
                        {
                            pWeather->temp_max = cJSON_GetNumberValue(jObj);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "low");
                        if (jObj && cJSON_IsNumber(jObj))
                        {
                            pWeather->temp_min = cJSON_GetNumberValue(jObj);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "dayCode");
                        if (jObj && cJSON_IsNumber(jObj))
                        {
                            pWeather->day_code = cJSON_GetNumberValue(jObj);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "dayText");
                        if (jObj && cJSON_IsString(jObj))
                        {
                            strncpy(pWeather->day_text, cJSON_GetStringValue(jObj), sizeof(pWeather->day_text) - 1);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "dayWindDirection");
                        if (jObj && cJSON_IsString(jObj))
                        {
                            strncpy(pWeather->day_wind_dir, cJSON_GetStringValue(jObj), sizeof(pWeather->day_wind_dir) - 1);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "dayWindScale");
                        if (jObj && cJSON_IsString(jObj))
                        {
                            strncpy(pWeather->day_wind_scale, cJSON_GetStringValue(jObj), sizeof(pWeather->day_wind_scale) - 1);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "nightCode");
                        if (jObj && cJSON_IsNumber(jObj))
                        {
                            pWeather->night_code = cJSON_GetNumberValue(jObj);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "nightText");
                        if (jObj && cJSON_IsString(jObj))
                        {
                            strncpy(pWeather->night_text, cJSON_GetStringValue(jObj), sizeof(pWeather->night_text) - 1);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "nightWindDirection");
                        if (jObj && cJSON_IsString(jObj))
                        {
                            strncpy(pWeather->night_wind_dir, cJSON_GetStringValue(jObj), sizeof(pWeather->night_wind_dir) - 1);
                        }

                        jObj = cJSON_GetObjectItem(jWeath, "nightWindScale");
                        if (jObj && cJSON_IsString(jObj))
                        {
                            strncpy(pWeather->night_wind_scale, cJSON_GetStringValue(jObj), sizeof(pWeather->night_wind_scale) - 1);
                        }

                        char key[12];
                        snprintf(key, sizeof(key), "w%d", i);
                        nvs_write_blob(NV_NAMESPACE_WEATHER, key, pWeather, sizeof(WEATHER));
                        ESP_LOGI(TAG, "weather %d saved", i);
                    }
                    free(pWeather);
                }
            }

            cJSON_free(root);
        } while (0);

        free(buffer);
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return err == ESP_OK;
}

static void time_sync_notification_cb(struct timeval *tv)
{
    struct tm timeinfo;
    char strftime_buf[64];
    last_sync_time = tv->tv_sec;
    localtime_r(&tv->tv_sec, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "SNTP synchronization done, time: %s", strftime_buf);
}

static void sync_task(void *pvParameter)
{
    WIFI_CONFIG cfg;
    if (nvs_read_wifi_config(&cfg))
    {
        app_wifi_init();
        if (app_wifi_connect(cfg.ssid, cfg.password))
        {
            app_sntp_sync_time(time_sync_notification_cb);
            if (app_sync_locatioin(&location))
            {

            }
        }
        app_wifi_disconnect();
        app_wifi_deinit();
    }

    vTaskDelete(NULL);
}

int sync_elapsed_time()
{
    return difftime(time(NULL), last_sync_time);
}

void cloud_sync()
{
    xTaskCreate(sync_task, "sync", 1024 * 10, NULL, tskIDLE_PRIORITY, NULL);
}

const LOCATION * app_get_locatioin()
{
    return location.city[0] ? &location : NULL;
}
