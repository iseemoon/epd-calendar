#pragma once

#include <stdbool.h>
#include <lvgl.h>
#include "esp_err.h"
#include "epd_panel_io.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct epd_panel_t epd_panel_t;         /*!< Type of EPD panel */
typedef epd_panel_t * epd_panel_handle_t;       /*!< Type of EPD panel handle */

typedef struct {
    const char * model;
    size_t width;
    size_t height;
    bool has_color;
} epd_panel_info_t;

/**
 * @brief Configuration structure for panel device
 */
typedef struct {
    int reset_gpio_num; /*!< GPIO used to reset the EPD panel, set to -1 if it's not used */
    int busy_gpio_num;  /*!< GPIO used for busy line, set to -1 if it's not used  */
    struct {
        unsigned int reset_active_high: 1;  /*!< Setting this if the panel reset is high level active */
        unsigned int busy_active_high: 1;   /*!< Setting this if the panel busy is high level active */
    } flags;                               /*!< EPD panel config flags */
    unsigned int bits_per_pixel;       /*!< Color depth, in bpp */
    void *vendor_config; /*!< vendor specific configuration, optional, left as NULL if not used */
} epd_panel_dev_config_t;


typedef enum {
    FLUSH_FULL,
    FLUSH_PARTIAL,
    FLUSH_PARTIAL_FAST,     // only support in bw mode
} EPD_FLUSH_MODE;

/**
 * @brief Create EPD panel for model
 *
 * @param[in] io EPD panel IO handle
 * @param[in] panel_dev_config general panel device configuration
 * @param[out] ret_panel Returned EPD panel handle
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_ERR_NO_MEM        if out of memory
 *          - ESP_OK                on success
 */
esp_err_t epd_new_panel(const epd_panel_io_handle_t io, const epd_panel_dev_config_t *panel_dev_config, epd_panel_handle_t *ret_panel);


/**
 * @brief Get base information of EPD panel
 *
 * @param[in] panel EPD panel handle, which is created by other factory API like `epd_new_panel()`
 * @param[out] panel_info Returned EPD panel information
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_ERR_NO_MEM        if out of memory
 *          - ESP_OK                on success
 */
esp_err_t epd_panel_get_info(epd_panel_handle_t panel, epd_panel_info_t * panel_info);

/**
 * @brief Reset EPD panel
 *
 * @note Panel reset must be called before attempting to initialize the panel using `epd_panel_init()`.
 *
 * @param[in] panel EPD panel handle, which is created by other factory API like `epd_new_panel()`
 * @return
 *          - ESP_OK on success
 */
esp_err_t epd_panel_reset(epd_panel_handle_t panel);

/**
 * @brief Initialize EPD panel
 *
 * @note Before calling this function, make sure the EPD panel has finished the `reset` stage by `epd_panel_reset()`.
 *
 * @param[in] panel EPD panel handle, which is created by other factory API like `epd_new_panel()`
 * @return
 *          - ESP_OK on success
 */
esp_err_t epd_panel_init(epd_panel_handle_t panel);

/**
 * @brief Deinitialize the EPD panel
 *
 * @param[in] panel EPD panel handle, which is created by other factory API like `epd_new_panel()`
 * @return
 *          - ESP_OK on success
 */
esp_err_t epd_panel_delete(epd_panel_handle_t panel);

esp_err_t epd_rounder(epd_panel_handle_t panel, lv_area_t * area);

/**
 * @brief Draw bitmap on EPD panel
 *
 * @param[in] panel EPD panel handle, which is created by other factory API like `epd_new_panel()`
 * @param[in] x_start Start index on x-axis (x_start included)
 * @param[in] y_start Start index on y-axis (y_start included)
 * @param[in] x_end End index on x-axis (x_end not included)
 * @param[in] y_end End index on y-axis (y_end not included)
 * @param[in] color_data color data that will be dumped to the specific window range
 * @return
 *          - ESP_OK on success
 */
esp_err_t epd_panel_flush(epd_panel_handle_t panel, const lv_area_t * area, bool is_last, const lv_color_t *color_data);


esp_err_t epd_rounder(epd_panel_handle_t panel, lv_area_t * area);

esp_err_t epd_set_px(epd_panel_handle_t panel, uint8_t * buf, lv_coord_t buf_w,
            lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa);

/**
 * @brief Turn on the display
 *
 * @param[in] panel EPD panel handle, which is created by other factory API like `epd_new_panel()`
 * @return
 *          - ESP_OK on success
 *          - ESP_ERR_NOT_SUPPORTED if this function is not supported by the panel
 */
esp_err_t epd_panel_power_on(epd_panel_handle_t panel);


/**
 * @brief Turn off the display
 *
 * @param[in] panel EPD panel handle, which is created by other factory API like `epd_new_panel()`
 * @return
 *          - ESP_OK on success
 *          - ESP_ERR_NOT_SUPPORTED if this function is not supported by the panel
 */
esp_err_t epd_panel_power_off(epd_panel_handle_t panel);


esp_err_t epd_set_flush_mode(epd_panel_handle_t panel, EPD_FLUSH_MODE mode);

#ifdef __cplusplus
}
#endif
