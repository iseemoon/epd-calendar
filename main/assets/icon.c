#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t upgrade_map[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x03, 0xff, 0xff, 0xc0, 
  0x06, 0x00, 0x00, 0x60, 
  0x04, 0x00, 0x00, 0x20, 
  0x04, 0x00, 0x00, 0x20, 
  0x04, 0x01, 0x80, 0x20, 
  0x04, 0x02, 0x40, 0x20, 
  0x04, 0x04, 0x20, 0x20, 
  0x04, 0x08, 0x10, 0x20, 
  0x04, 0x1c, 0x38, 0x20, 
  0x04, 0x3e, 0x3c, 0x20, 
  0x04, 0x06, 0x20, 0x20, 
  0x04, 0x07, 0xe0, 0x20, 
  0x04, 0x00, 0x00, 0x20, 
  0x04, 0x00, 0x00, 0x20, 
  0x04, 0x03, 0xc0, 0x20, 
  0x04, 0x00, 0x00, 0x20, 
  0x04, 0x03, 0xc0, 0x20, 
  0x04, 0x00, 0x00, 0x20, 
  0x04, 0x00, 0x00, 0x20, 
  0x04, 0x00, 0x00, 0x20, 
  0x06, 0x00, 0x00, 0x60, 
  0x03, 0xff, 0xff, 0xc0, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
};

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t wifi_map[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x3f, 0xf8, 0x00, 
  0x00, 0xff, 0xff, 0x00, 
  0x03, 0xfe, 0x3f, 0xc0, 
  0x0f, 0xc0, 0x03, 0xf0, 
  0x1f, 0x00, 0x00, 0x78, 
  0x3c, 0x00, 0x00, 0x3c, 
  0x38, 0x0f, 0xe0, 0x1c, 
  0x00, 0x3f, 0xfc, 0x04, 
  0x00, 0xff, 0xff, 0x00, 
  0x01, 0xf8, 0x0f, 0x80, 
  0x03, 0xc0, 0x03, 0xc0, 
  0x03, 0x80, 0x01, 0xc0, 
  0x00, 0x07, 0xe0, 0x00, 
  0x00, 0x0f, 0xf8, 0x00, 
  0x00, 0x3f, 0xfc, 0x00, 
  0x00, 0x38, 0x1c, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x01, 0x80, 0x00, 
  0x00, 0x03, 0xc0, 0x00, 
  0x00, 0x01, 0xc0, 0x00, 
  0x00, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
};

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t gallery_map[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x0f, 0xff, 0xff, 0xf0, 
  0x1f, 0xff, 0xff, 0xf8, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x0f, 0x0c, 
  0x30, 0x00, 0x09, 0x8c, 
  0x30, 0x00, 0x18, 0xcc, 
  0x30, 0x00, 0x18, 0x8c, 
  0x30, 0x00, 0x0f, 0x8c, 
  0x30, 0x00, 0x07, 0x0c, 
  0x30, 0x40, 0x00, 0x0c, 
  0x31, 0xe0, 0x00, 0x0c, 
  0x33, 0xb8, 0x78, 0x0c, 
  0x3e, 0x0c, 0xec, 0x0c, 
  0x3c, 0x07, 0x87, 0x0c, 
  0x30, 0x03, 0x01, 0x8c, 
  0x30, 0x00, 0x00, 0xec, 
  0x30, 0x00, 0x00, 0x3c, 
  0x30, 0x00, 0x00, 0x1c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x1f, 0xff, 0xff, 0xf8, 
  0x0f, 0xff, 0xff, 0xf0, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
};

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t dig_colck_map[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x0f, 0xff, 0xff, 0xf8, 
  0x3f, 0xff, 0xff, 0xf8, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x08, 0x0a, 0x0c, 
  0x31, 0x04, 0x4a, 0x4c, 
  0x31, 0x04, 0x4a, 0x4c, 
  0x31, 0x04, 0x4a, 0x4c, 
  0x31, 0x04, 0x4a, 0x4c, 
  0x30, 0x20, 0x08, 0x4c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x1f, 0xff, 0xff, 0xfc, 
  0x1f, 0xff, 0xff, 0xf0, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
};

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t clock_map[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x1d, 0xb0, 0x00, 
  0x00, 0x69, 0x8c, 0x00, 
  0x00, 0xe0, 0x07, 0x00, 
  0x01, 0xa0, 0x05, 0x80, 
  0x03, 0x10, 0x00, 0xc0, 
  0x06, 0x00, 0x20, 0x60, 
  0x04, 0x00, 0x00, 0x30, 
  0x0e, 0x00, 0x40, 0x70, 
  0x08, 0x00, 0x40, 0x88, 
  0x10, 0x00, 0x00, 0x18, 
  0x10, 0x00, 0x80, 0x08, 
  0x00, 0x00, 0x80, 0x00, 
  0x18, 0x01, 0x80, 0x18, 
  0x18, 0x0f, 0x80, 0x18, 
  0x00, 0x1c, 0x40, 0x00, 
  0x10, 0x00, 0x20, 0x08, 
  0x18, 0x00, 0x10, 0x08, 
  0x11, 0x00, 0x00, 0x10, 
  0x0e, 0x00, 0x00, 0x70, 
  0x0c, 0x00, 0x00, 0x20, 
  0x06, 0x00, 0x00, 0x60, 
  0x03, 0x00, 0x08, 0xc0, 
  0x01, 0xa0, 0x05, 0x80, 
  0x00, 0xe0, 0x07, 0x00, 
  0x00, 0x31, 0x96, 0x00, 
  0x00, 0x0d, 0xb8, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
};

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t calendar1_map[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x80, 0x01, 0x00, 
  0x00, 0x80, 0x01, 0x00, 
  0x0f, 0xff, 0xff, 0xe0, 
  0x1f, 0xff, 0xff, 0xf8, 
  0x10, 0x80, 0x01, 0x0c, 
  0x30, 0x80, 0x01, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x3f, 0xff, 0xff, 0xfc, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x38, 0x18, 0x0c, 
  0x30, 0x7c, 0x36, 0x0c, 
  0x30, 0x46, 0x42, 0x0c, 
  0x30, 0xc2, 0x62, 0x0c, 
  0x30, 0x02, 0x26, 0x0c, 
  0x30, 0x04, 0x3c, 0x0c, 
  0x30, 0x18, 0x62, 0x0c, 
  0x30, 0x30, 0x43, 0x0c, 
  0x30, 0x60, 0x43, 0x0c, 
  0x30, 0x40, 0x62, 0x0c, 
  0x30, 0xfe, 0x3e, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x08, 
  0x1f, 0xff, 0xff, 0xf8, 
  0x07, 0xff, 0xff, 0xf0, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
};

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t calendar_map[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x30, 0x0c, 0x00, 
  0x1f, 0xff, 0xff, 0xf0, 
  0x18, 0x30, 0x0c, 0x0c, 
  0x30, 0x30, 0x0c, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x3f, 0xff, 0xff, 0xfc, 
  0x3f, 0xff, 0xff, 0xfc, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x33, 0xc3, 0xc3, 0xcc, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x33, 0xc3, 0xc3, 0xcc, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x33, 0xc3, 0xc3, 0xcc, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x30, 0x00, 0x00, 0x0c, 
  0x1f, 0xff, 0xff, 0xf8, 
  0x0f, 0xff, 0xff, 0xf0, 
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
};

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t reset_map[] = {
  0x00, 0x18, 0x00, 0x00, 
  0x00, 0x3e, 0x00, 0x00, 
  0x00, 0x3f, 0x18, 0x00, 
  0x00, 0x3f, 0x3e, 0x00, 
  0x00, 0x7f, 0x1f, 0x00, 
  0x01, 0xff, 0x0f, 0xc0, 
  0x03, 0xfe, 0x03, 0xc0, 
  0x07, 0x8e, 0x01, 0xe0, 
  0x0f, 0x04, 0x00, 0xf0, 
  0x0e, 0x00, 0x00, 0x70, 
  0x1e, 0x00, 0x00, 0x38, 
  0x1c, 0x00, 0x00, 0x38, 
  0x3c, 0x00, 0x00, 0x3c, 
  0x38, 0x00, 0x00, 0x1c, 
  0x38, 0x00, 0x00, 0x1c, 
  0x38, 0x00, 0x00, 0x1c, 
  0x38, 0x00, 0x00, 0x1c, 
  0x38, 0x00, 0x00, 0x1c, 
  0x38, 0x00, 0x00, 0x3c, 
  0x1c, 0x00, 0x00, 0x38, 
  0x1c, 0x00, 0x00, 0x38, 
  0x1e, 0x00, 0x00, 0x78, 
  0x0e, 0x00, 0x00, 0xf0, 
  0x0f, 0x00, 0x38, 0xe0, 
  0x07, 0x80, 0x3f, 0xc0, 
  0x03, 0xe0, 0x7f, 0xc0, 
  0x01, 0xf8, 0xff, 0x00, 
  0x00, 0xfc, 0xfe, 0x00, 
  0x00, 0x3c, 0xfc, 0x00, 
  0x00, 0x00, 0x7c, 0x00, 
  0x00, 0x00, 0x3e, 0x00, 
  0x00, 0x00, 0x0c, 0x00, 
};

const lv_img_dsc_t img_calendar = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 32,
  .header.h = 32,
  .data_size = 128,
  .data = calendar_map,
};

const lv_img_dsc_t img_calendar1 = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 32,
  .header.h = 32,
  .data_size = 128,
  .data = calendar1_map,
};

const lv_img_dsc_t img_clock = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 32,
  .header.h = 32,
  .data_size = 128,
  .data = clock_map,
};

const lv_img_dsc_t img_dig_colck = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 32,
  .header.h = 32,
  .data_size = 128,
  .data = dig_colck_map,
};

const lv_img_dsc_t img_gallery = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 32,
  .header.h = 32,
  .data_size = 128,
  .data = gallery_map,
};

const lv_img_dsc_t img_reset = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 32,
  .header.h = 32,
  .data_size = 128,
  .data = reset_map,
};

const lv_img_dsc_t img_upgrade = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 32,
  .header.h = 32,
  .data_size = 128,
  .data = upgrade_map,
};

const lv_img_dsc_t img_wifi = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 32,
  .header.h = 32,
  .data_size = 128,
  .data = wifi_map,
};
