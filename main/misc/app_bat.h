#pragma once

#ifdef __cplusplus
extern "C" {
#endif


int get_battery_vol();

float get_battery_percent(int voltage);

#ifdef __cplusplus
}
#endif
