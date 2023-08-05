#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif


#define NV_NAMESPACE_WIFI       "wifi"
#define NV_KEY_SSID             "ssid"
#define NV_KEY_PASSWD           "passwd"

#define NV_NAMESPACE_SYS        "system"
#define NV_KEY_SCREEN           "screen"

#define NV_NAMESPACE_WEATHER    "weather"

typedef struct _WIFI_CONFIG {
    char ssid[32];
    char password[64];
} WIFI_CONFIG;

void nvs_clear_all();

bool nvs_read_wifi_config(WIFI_CONFIG * cfg);
esp_err_t nvs_save_wifi_config(const WIFI_CONFIG * cfg);

bool wait_for_wifi_config(uint32_t ms);

bool nvs_set_screen(uint8_t n);

uint8_t nvs_get_screen();

bool nvs_write_blob(const char * namespace, const char * key, void * data, size_t len);

bool nvs_read_blob(const char * namespace, const char * key, void * data, size_t len);

void nvs_erase_item(const char * namespace, const char * key);

void nvs_erase_space(const char * namespace);

#ifdef __cplusplus
}
#endif
