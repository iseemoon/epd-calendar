#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t start_http_server(const char *base_path);

#ifdef __cplusplus
}
#endif
