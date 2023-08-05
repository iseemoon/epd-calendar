#pragma once
#ifndef __APP_WIFI_H__
#define __APP_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

bool app_wifi_init();
void app_wifi_deinit();

const char * get_ap_ssid();
const char * get_ap_password();

void wifi_init_softap(void);

bool app_wifi_connect(const char * ssid, const char * password);
bool app_wifi_connected(void);
uint32_t app_wifi_get_ip(void);
void app_wifi_disconnect(void);



#ifdef __cplusplus
}
#endif

#endif	// __APP_WIFI_H__
