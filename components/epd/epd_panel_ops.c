#include "esp_check.h"
#include "epd_panel_ops.h"
#include "internal/epd_panel_interface.h"

#define CONFIG_EPD_GDEY075Z08

#if defined CONFIG_EPD_GDEY075Z08
#include "internal/gdey075.h"
#endif

static const char *TAG = "epd_panel";


esp_err_t epd_new_panel(const epd_panel_io_handle_t io, const epd_panel_dev_config_t *panel_dev_config, epd_panel_handle_t *ret_panel)
{
#if defined CONFIG_EPD_GDEY075Z08
    return epd_new_panel_gdey075(io, panel_dev_config, ret_panel);
#endif
    return ESP_FAIL;
}

esp_err_t epd_panel_get_info(epd_panel_handle_t panel, epd_panel_info_t * panel_info)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->get_info(panel, panel_info);
}

esp_err_t epd_panel_reset(epd_panel_handle_t panel)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->reset(panel);
}

esp_err_t epd_panel_init(epd_panel_handle_t panel)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->init(panel);
}

esp_err_t epd_panel_delete(epd_panel_handle_t panel)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->del(panel);
}

esp_err_t epd_panel_flush(epd_panel_handle_t panel, const lv_area_t * area, bool is_last, const lv_color_t *color_data)
{
    // assert((x1 >= 0 && y1 >= 0) && "start position can NOT be smaller than zero");
    // assert((x2 > x1 && y2 > y1) && "the region size to flush must greater than zero");
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->flush(panel, area, is_last, color_data);
}

esp_err_t epd_rounder(epd_panel_handle_t panel, lv_area_t * area)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->rounder(panel, area);
}

esp_err_t epd_set_px(epd_panel_handle_t panel, uint8_t * buf, lv_coord_t buf_w,
            lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->set_px(panel, buf, buf_w, x, y, color, opa);
}

esp_err_t epd_panel_power_on(epd_panel_handle_t panel)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->power_on(panel);
}

esp_err_t epd_panel_deep_sleep(epd_panel_handle_t panel)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->deep_sleep(panel);
}

esp_err_t epd_set_flush_mode(epd_panel_handle_t panel, EPD_FLUSH_MODE mode)
{
    ESP_RETURN_ON_FALSE(panel, ESP_ERR_INVALID_ARG, TAG, "invalid panel handle");
    return panel->set_flush_mode(panel, mode);
}
