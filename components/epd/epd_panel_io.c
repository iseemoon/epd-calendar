#include "esp_check.h"
#include "epd_panel_io.h"


static const char *TAG = "epd_panel.io";

esp_err_t epd_panel_io_rx_param(epd_panel_io_handle_t io, int lcd_cmd, void *param, size_t param_size)
{
    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_ARG, TAG, "invalid panel io handle");
    ESP_RETURN_ON_FALSE(io->rx_param, ESP_ERR_NOT_SUPPORTED, TAG, "rx_param is not supported yet");
    return io->rx_param(io, lcd_cmd, param, param_size);
}

esp_err_t epd_panel_io_tx_param(epd_panel_io_handle_t io, int lcd_cmd, const void *param, size_t param_size)
{
    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_ARG, TAG, "invalid panel io handle");
    return io->tx_param(io, lcd_cmd, param, param_size);
}

esp_err_t epd_panel_io_tx_color(epd_panel_io_handle_t io, int lcd_cmd, const void *color, size_t color_size)
{
    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_ARG, TAG, "invalid panel io handle");
    return io->tx_color(io, lcd_cmd, color, color_size);
}

esp_err_t epd_panel_io_del(epd_panel_io_handle_t io)
{
    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_ARG, TAG, "invalid panel io handle");
    return io->del(io);
}

esp_err_t epd_panel_io_register_event_callbacks(epd_panel_io_handle_t io, const epd_panel_io_callbacks_t *cbs, void *user_ctx)
{
    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_ARG, TAG, "invalid panel io handle");
    ESP_RETURN_ON_FALSE(cbs, ESP_ERR_INVALID_ARG, TAG, "invalid callbacks structure");
    return io->register_event_callbacks(io, cbs, user_ctx);
}
