#pragma once
#ifndef _SHT30_
#define _SHT30_

bool sht30_init(void);

bool sht30_get_value(uint8_t * temp, uint8_t * humi);

#endif