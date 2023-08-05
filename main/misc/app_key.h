#ifndef __USER_KEY_H__
#define __USER_KEY_H__

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_NUM             3

#define GPIO_KEY2           GPIO_NUM_34
#define GPIO_KEY3           GPIO_NUM_35
#define GPIO_KEY4           GPIO_NUM_39

#define GPIO_KEY_MASK       ((1ULL<<GPIO_KEY2) | (1ULL<<GPIO_KEY3) | (1ULL<<GPIO_KEY4))

typedef enum _KEY_EVENT
{
    KEY_EVT_DOWN = 0x81,
    KEY_EVT_HOLDING = 0x82,
    KEY_EVT_UP = 0x83
} KEY_EVENT;


typedef void (*key_event_handler)(gpio_num_t gpio, KEY_EVENT event);


void user_key_init(key_event_handler key_hanlder);

#ifdef __cplusplus
}
#endif

#endif	// __USER_KEY_H__
