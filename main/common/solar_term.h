#pragma once

#include "date.h"

#ifdef __cplusplus
extern "C" {
#endif

// 计算某年的第n个节气为几日(从小寒开始，n取值范围：0～23)
// y >= 1900
const char * get_day_of_solar_term(uint32_t y, uint8_t n, DATE * solar_date);

#ifdef __cplusplus
}
#endif