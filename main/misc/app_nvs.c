#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include "app_nvs.h"
#include "esp_log.h"


static const char * TAG = "nvs";

static EventGroupHandle_t   s_nvs_event_group = NULL;

#define NVS_BIT_WIFI_CFG    BIT0


void nvs_clear_all()
{
    nvs_handle_t handle;
    if (nvs_open(NV_NAMESPACE_WIFI, NVS_READWRITE, &handle) == ESP_OK)
    {
        nvs_erase_all(handle);
        nvs_commit(handle);
        nvs_close(handle);
    }

    if (nvs_open(NV_NAMESPACE_SYS, NVS_READWRITE, &handle) == ESP_OK)
    {
        nvs_erase_all(handle);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

esp_err_t nvs_save_wifi_config(const WIFI_CONFIG * cfg)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NV_NAMESPACE_WIFI, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Updating ssid and password...");
        err = nvs_set_str(my_handle, NV_KEY_SSID, cfg->ssid);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Fail to save ssid, %s", esp_err_to_name(err));
        }
        else {
            err = nvs_set_str(my_handle, NV_KEY_PASSWD, cfg->password);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Fail to save password, %s", esp_err_to_name(err));
            }
            else {
                err = nvs_commit(my_handle);
                if (err == ESP_OK && s_nvs_event_group)
                {
                    xEventGroupSetBits(s_nvs_event_group, NVS_BIT_WIFI_CFG);
                }
                ESP_LOGI(TAG, "wifi config %s", (err != ESP_OK) ? "Failed" : "Done");
            }
        }
        nvs_close(my_handle);
    }
    return err;
}

bool nvs_read_wifi_config(WIFI_CONFIG * cfg)
{
    bool ret = false;
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NV_NAMESPACE_WIFI, NVS_READONLY, &handle);
    if (err == ESP_OK)
    {
        if (cfg != NULL)
        {
            ESP_LOGI(TAG, "reading ssid and password...");
            size_t len = sizeof(cfg->ssid);
            err = nvs_get_str(handle, NV_KEY_SSID, cfg->ssid, &len);
            if (err == ESP_OK)
            {
                len = sizeof(cfg->password);
                ret = nvs_get_str(handle, NV_KEY_PASSWD, cfg->password, &len) == ESP_OK;
            }
        }
        else
        {
            ret = nvs_get_str(handle, NV_KEY_PASSWD, NULL, NULL) == ESP_ERR_NVS_INVALID_LENGTH;
            ret = ret && nvs_get_str(handle, NV_KEY_SSID, NULL, NULL) == ESP_ERR_NVS_INVALID_LENGTH;
        }
        nvs_close(handle);
    }
    else
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle", esp_err_to_name(err));
    }
    return ret;
}

bool wait_for_wifi_config(uint32_t ms)
{
    if (!s_nvs_event_group)
    {
        s_nvs_event_group = xEventGroupCreate();
    }

    EventBits_t bits = xEventGroupWaitBits(s_nvs_event_group,
            NVS_BIT_WIFI_CFG,
            pdFALSE,
            pdFALSE,
            ms == UINT32_MAX ? portMAX_DELAY : pdMS_TO_TICKS(ms));

    return (bits & NVS_BIT_WIFI_CFG);
}

bool nvs_set_screen(uint8_t n)
{
    nvs_handle_t handle;
    if (nvs_open(NV_NAMESPACE_SYS, NVS_READWRITE, &handle) == ESP_OK)
    {
        if (nvs_set_u8(handle, NV_KEY_SCREEN, n) == ESP_OK)
        {
            nvs_commit(handle);
            nvs_close(handle);
            return true;
        }
        nvs_close(handle);
    }
    return false;
}

uint8_t nvs_get_screen()
{
    uint8_t n = 0;
    nvs_handle_t handle;
    if (nvs_open(NV_NAMESPACE_SYS, NVS_READONLY, &handle) == ESP_OK)
    {
        if (nvs_get_u8(handle, NV_KEY_SCREEN, &n) != ESP_OK)
        {
            ESP_LOGI(TAG, "Screen not set");
        }
        nvs_close(handle);
    }
    return n;
}

bool nvs_write_blob(const char * namespace, const char * key, void * data, size_t len)
{
    nvs_handle_t handle;
    if (nvs_open(namespace, NVS_READWRITE, &handle) == ESP_OK)
    {
        if (nvs_set_blob(handle, key, data, len) == ESP_OK)
        {
            nvs_commit(handle);
            nvs_close(handle);
            return true;
        }
        nvs_close(handle);
    }
    return false;
}

bool nvs_read_blob(const char * namespace, const char * key, void * data, size_t len)
{
    bool ret = false;
    nvs_handle_t handle;
    if (nvs_open(namespace, NVS_READONLY, &handle) == ESP_OK)
    {
        if (nvs_get_blob(handle, key, data, &len) == ESP_OK)
        {
            ret = true;
        }
        nvs_close(handle);
    }
    return ret;
}

void nvs_erase_item(const char * namespace, const char * key)
{
    nvs_handle_t handle;
    if (nvs_open(namespace, NVS_READWRITE, &handle) == ESP_OK)
    {
        nvs_erase_key(handle, key);
        nvs_close(handle);
    }
}

void nvs_erase_space(const char * namespace)
{
    nvs_handle_t handle;
    if (nvs_open(namespace, NVS_READWRITE, &handle) == ESP_OK)
    {
        nvs_erase_all(handle);
        nvs_close(handle);
    }
}
