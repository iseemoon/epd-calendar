#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "app_key.h"


#define KEY_LOG_TAG             "key"
#define ESP_INTR_FLAG_DEFAULT   0


typedef struct _KEY_DATA {
    gpio_num_t      gpio_num;
    int64_t         press_time;      // elapse time in micro second since boot when the key was pressed
} KEY_DATA;


static KEY_DATA keys[KEY_NUM] = { 0 };

static QueueHandle_t gpio_evt_queue = NULL;

static key_event_handler m_key_event_handler = NULL;


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t index = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &index, NULL);
}

static void key_process_task(void* arg)
{
    uint32_t idx;
    int ret = 0;
    bool key_pressed = false;

    printf("key task entered\n");

    for(;;)
    {
        ret = xQueueReceive(gpio_evt_queue, &idx, key_pressed ? pdMS_TO_TICKS(100) : portMAX_DELAY);
        if (ret && idx < KEY_NUM)
        {
            gpio_num_t gpio_num = keys[idx].gpio_num;
            int level = gpio_get_level(gpio_num);
            if (!level && keys[idx].press_time == 0)      // key pressed, pull up the pin to high level
            {
                keys[idx].press_time = esp_timer_get_time();
                gpio_set_intr_type(gpio_num, GPIO_INTR_POSEDGE);
                m_key_event_handler(gpio_num, KEY_EVT_DOWN);
            }
            else if (level && keys[idx].press_time > 0)
            {
                keys[idx].press_time = 0;
                gpio_set_intr_type(gpio_num, GPIO_INTR_NEGEDGE);
                m_key_event_handler(gpio_num, KEY_EVT_UP);
            }
        }

        key_pressed = false;
        int64_t now = esp_timer_get_time();
        for (uint8_t i=0; i < KEY_NUM; ++i)
        {
            key_pressed = key_pressed || keys[i].press_time > 0;
            if (keys[i].press_time && now - keys[i].press_time >= 1000000)
            {
                m_key_event_handler(keys[i].gpio_num, KEY_EVT_HOLDING);
            }
        }
    }
}


/******************************************************************************
 * FunctionName : user_light_set_duty
 * Description  : set each channel's duty params
 * Parameters   : uint8 duty    : 0 ~ PWM_DEPTH
 *                uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
void user_key_init(key_event_handler key_hanlder)
{
    gpio_config_t io_conf = { 
        .intr_type = GPIO_INTR_NEGEDGE,  // interrupt of both rising and falling edge
        .pin_bit_mask = GPIO_KEY_MASK,   // bit mask of the pins
        .mode = GPIO_MODE_INPUT,         // set as input mode
        // .pull_down_en = 0,            // enable pull-down mode
        // .pull_up_en = 1,              // disable pull-up mode 
    };
    gpio_config(&io_conf);

    memset(&keys, 0, sizeof(keys));
    keys[0].gpio_num = GPIO_KEY2;
    keys[1].gpio_num = GPIO_KEY3;
    keys[2].gpio_num = GPIO_KEY4;

    m_key_event_handler = key_hanlder;

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(key_process_task, "key_task", 4096, NULL, 10, NULL);

    //install gpio isr service
    esp_err_t err = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_KEY2, gpio_isr_handler, (void*) 0);
    gpio_isr_handler_add(GPIO_KEY3, gpio_isr_handler, (void*) 1);
    gpio_isr_handler_add(GPIO_KEY4, gpio_isr_handler, (void*) 2);
}

