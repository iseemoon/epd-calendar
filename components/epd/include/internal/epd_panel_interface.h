#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include <lvgl.h>
#include "../epd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct epd_panel_info_t * pepd_panel_info_t;

/**
 * @brief EPD panel interface
 */
struct epd_panel_t {
    /**
     * @brief Reset EPD panel
     *
     * @param[in] panel EPD panel handle, which is created by other factory
     * @return
     *          - ESP_OK on success
     */
    esp_err_t (*reset)(struct epd_panel_t *panel);

    /**
     * @brief Initialize EPD panel
     *
     * @param[in] panel EPD panel handle, which is created by other factory
     * @return
     *          - ESP_OK on success
     */
    esp_err_t (*init)(struct epd_panel_t *panel);

    /**
     * @brief Destory EPD panel
     *
     * @param[in] panel EPD panel handle, which is created by other factory
     * @return
     *          - ESP_OK on success
     */
    esp_err_t (*del)(struct epd_panel_t *panel);


    esp_err_t (*get_info)(struct epd_panel_t *panel, pepd_panel_info_t panel_info);

    /**
     * @brief Draw bitmap on EPD panel
     *
     * @param[in] panel EPD panel handle, which is created by other factory API
     * @param[in] x_start Start index on x-axis (x_start included)
     * @param[in] y_start Start index on y-axis (y_start included)
     * @param[in] x_end End index on x-axis (x_end not included)
     * @param[in] y_end End index on y-axis (y_end not included)
     * @param[in] color_data RGB color data that will be dumped to the specific window range
     * @return
     *          - ESP_OK on success
     */
    esp_err_t (*flush)(struct epd_panel_t *panel, const lv_area_t * area, bool is_last, const void *color_data);


    esp_err_t (*rounder)(struct epd_panel_t *panel, lv_area_t * area);


    esp_err_t (*set_px)(struct epd_panel_t *panel, uint8_t * buf, lv_coord_t buf_w,
            lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa);
    

    esp_err_t (*set_flush_mode)(struct epd_panel_t *panel, EPD_FLUSH_MODE mode);

    /**
     * @brief Turn on the display
     *
     * @param[in] panel EPD panel handle, which is created by other factory API
     * @return
     *          - ESP_OK on success
     *          - ESP_ERR_NOT_SUPPORTED if this function is not supported by the panel
     */
    esp_err_t (*power_on)(struct epd_panel_t *panel);

    /**
     * @brief light sleep
     *
     * @param[in] panel EPD panel handle, which is created by other factory API
     * @return
     *          - ESP_OK on success
     *          - ESP_ERR_NOT_SUPPORTED if this function is not supported by the panel
     */
    esp_err_t (*light_sleep)(struct epd_panel_t *panel);

        /**
     * @brief set epd to Deep Sleep Mode to save power
     *
     * @param[in] panel EPD panel handle, which is created by other factory API
     * @return
     *          - ESP_OK on success
     *          - ESP_ERR_NOT_SUPPORTED if this function is not supported by the panel
     */
    esp_err_t (*deep_sleep)(struct epd_panel_t *panel);
};

#ifdef __cplusplus
}
#endif
