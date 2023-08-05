/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "soc/soc_caps.h"
#include "hal/spi_types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct epd_panel_io_t epd_panel_io_t;               /*!< Type of EPD panel IO */
typedef struct epd_panel_io_t * epd_panel_io_handle_t;      /*!< Type of EPD panel IO handle */

/**
 * @brief Type of LCD panel IO event data
 */
typedef struct {
} epd_panel_io_event_data_t;

/**
 * @brief Declare the prototype of the function that will be invoked when panel IO finishes transferring color data
 *
 * @param[in] panel_io EPD panel IO handle, which is created by factory API like `epd_new_panel_io_spi()`
 * @param[in] edata Panel IO event data, fed by driver
 * @param[in] user_ctx User data, passed from `epd_panel_io_xxx_config_t`
 * @return Whether a high priority task has been waken up by this function
 */
typedef bool (*epd_panel_io_color_trans_done_cb_t)(epd_panel_io_handle_t panel_io, epd_panel_io_event_data_t *edata, void *user_ctx);

/**
 * @brief Type of EPD panel IO callbacks
 */
typedef struct {
    epd_panel_io_color_trans_done_cb_t on_color_trans_done; /*!< Callback invoked when color data transfer has finished */
} epd_panel_io_callbacks_t;


/**
 * @brief EPD panel IO interface
 */
struct epd_panel_io_t {
    /**
     * @brief Transmit EPD command and receive corresponding parameters
     *
     * @note This is the panel-specific interface called by function `esp_lcd_panel_io_rx_param()`.
     *
     * @param[in]  io EPD panel IO handle, which is created by other factory API like `esp_lcd_new_panel_io_spi()`
     * @param[in]  cmd The specific EPD command, set to -1 if no command needed
     * @param[out] param Buffer for the command data
     * @param[in]  param_size Size of `param` buffer
     * @return
     *          - ESP_ERR_INVALID_ARG   if parameter is invalid
     *          - ESP_ERR_NOT_SUPPORTED if read is not supported by transport
     *          - ESP_OK                on success
     */
    esp_err_t (*rx_param)(epd_panel_io_t *io, int cmd, void *param, size_t param_size);

    /**
     * @brief Transmit EPD command and corresponding parameters
     *
     * @note This is the panel-specific interface called by function `esp_lcd_panel_io_tx_param()`.
     *
     * @param[in] io EPD panel IO handle, which is created by other factory API like `esp_lcd_new_panel_io_spi()`
     * @param[in] lcd_cmd The specific LCD command
     * @param[in] param Buffer that holds the command specific parameters, set to NULL if no parameter is needed for the command
     * @param[in] param_size Size of `param` in memory, in bytes, set to zero if no parameter is needed for the command
     * @return
     *          - ESP_ERR_INVALID_ARG   if parameter is invalid
     *          - ESP_OK                on success
     */
    esp_err_t (*tx_param)(epd_panel_io_t *io, int cmd, const void *param, size_t param_size);

    /**
     * @brief Transmit EPD RGB data
     *
     * @note This is the panel-specific interface called by function `esp_lcd_panel_io_tx_color()`.
     *
     * @param[in] io EPD panel IO handle, which is created by other factory API like `esp_lcd_new_panel_io_spi()`
     * @param[in] lcd_cmd The specific EPD command
     * @param[in] color Buffer that holds the RGB color data
     * @param[in] color_size Size of `color` in memory, in bytes
     * @return
     *          - ESP_ERR_INVALID_ARG   if parameter is invalid
     *          - ESP_OK                on success
     */
    esp_err_t (*tx_color)(epd_panel_io_t *io, int cmd, const void *color, size_t color_size);

    /**
     * @brief Destory EPD panel IO handle (deinitialize all and free resource)
     *
     * @param[in] io EPD panel IO handle, which is created by other factory API like `esp_lcd_new_panel_io_spi()`
     * @return
     *          - ESP_ERR_INVALID_ARG   if parameter is invalid
     *          - ESP_OK                on success
     */
    esp_err_t (*del)(epd_panel_io_t *io);

    /**
     * @brief Register EPD panel IO callbacks
     *
     * @param[in] io EPD panel IO handle, which is created by factory API like `esp_lcd_new_panel_io_spi()`
     * @param[in] cbs structure with all LCD panel IO callbacks
     * @param[in] user_ctx User private data, passed directly to callback's user_ctx
     * @return
     *          - ESP_ERR_INVALID_ARG   if parameter is invalid
     *          - ESP_OK                on success
     */
    esp_err_t (*register_event_callbacks)(epd_panel_io_t *io, const epd_panel_io_callbacks_t *cbs, void *user_ctx);
};


/**
 * @brief Transmit EPD command and receive corresponding parameters
 *
 * @note Commands sent by this function are short, so they are sent using polling transactions.
 *       The function does not return before the command transfer is completed.
 *       If any queued transactions sent by `epd_panel_io_tx_color()` are still pending when this function is called,
 *       this function will wait until they are finished and the queue is empty before sending the command(s).
 *
 * @param[in]  io EPD panel IO handle, which is created by other factory API like `epd_new_panel_io_spi()`
 * @param[in]  cmd The specific EPD command, set to -1 if no command needed
 * @param[out] param Buffer for the command data
 * @param[in]  param_size Size of `param` buffer
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_ERR_NOT_SUPPORTED if read is not supported by transport
 *          - ESP_OK                on success
 */
esp_err_t epd_panel_io_rx_param(epd_panel_io_handle_t io, int cmd, void *param, size_t param_size);

/**
 * @brief Transmit EPD command and corresponding parameters
 *
 * @note Commands sent by this function are short, so they are sent using polling transactions.
 *       The function does not return before the command transfer is completed.
 *       If any queued transactions sent by `epd_panel_io_tx_color()` are still pending when this function is called,
 *       this function will wait until they are finished and the queue is empty before sending the command(s).
 *
 * @param[in] io EPD panel IO handle, which is created by other factory API like `epd_new_panel_io_spi()`
 * @param[in] lcd_cmd The specific EPD command (set to -1 if no command needed - only in SPI and I2C)
 * @param[in] param Buffer that holds the command specific parameters, set to NULL if no parameter is needed for the command
 * @param[in] param_size Size of `param` in memory, in bytes, set to zero if no parameter is needed for the command
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_OK                on success
 */
esp_err_t epd_panel_io_tx_param(epd_panel_io_handle_t io, int cmd, const void *param, size_t param_size);

/**
 * @brief Transmit EPD RGB data
 *
 * @note This function will package the command and RGB data into a transaction, and push into a queue.
 *       The real transmission is performed in the background (DMA+interrupt).
 *       The caller should take care of the lifecycle of the `color` buffer.
 *       Recycling of color buffer should be done in the callback `on_color_trans_done()`.
 *
 * @param[in] io LCD panel IO handle, which is created by factory API like `esp_lcd_new_panel_io_spi()`
 * @param[in] lcd_cmd The specific LCD command, set to -1 if no command needed
 * @param[in] color Buffer that holds the RGB color data
 * @param[in] color_size Size of `color` in memory, in bytes
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_OK                on success
 */
esp_err_t epd_panel_io_tx_color(epd_panel_io_handle_t io, int cmd, const void *color, size_t color_size);

/**
 * @brief Destroy EPD panel IO handle (deinitialize panel and free all corresponding resource)
 *
 * @param[in] io EDP panel IO handle, which is created by factory API like `epd_new_panel_io_spi()`
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_OK                on success
 */
esp_err_t epd_panel_io_del(epd_panel_io_handle_t io);

/**
 * @brief Register EPD panel IO callbacks
 *
 * @param[in] io EPD panel IO handle, which is created by factory API like `epd_new_panel_io_spi()`
 * @param[in] cbs structure with all EPD panel IO callbacks
 * @param[in] user_ctx User private data, passed directly to callback's user_ctx
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_OK                on success
 */
esp_err_t epd_panel_io_register_event_callbacks(epd_panel_io_handle_t io, const epd_panel_io_callbacks_t *cbs, void *user_ctx);

/**
 * @brief Panel IO configuration structure, for SPI interface
 */
typedef struct {
    int cs_gpio_num;    /*!< GPIO used for CS line */
    int dc_gpio_num;    /*!< GPIO used to select the D/C line, set this to -1 if the D/C line is not used */
    int spi_mode;       /*!< Traditional SPI mode (0~3) */
    unsigned int pclk_hz;    /*!< Frequency of pixel clock */
    size_t trans_queue_depth; /*!< Size of internal transaction queue */
    epd_panel_io_color_trans_done_cb_t on_color_trans_done; /*!< Callback invoked when color data transfer has finished */
    void *user_ctx;    /*!< User private data, passed directly to on_color_trans_done's user_ctx */
    int epd_cmd_bits;   /*!< Bit-width of EPD command */
    int epd_param_bits; /*!< Bit-width of EPD parameter */
    struct {
        unsigned int dc_low_on_data: 1;  /*!< If this flag is enabled, DC line = 0 means transfer data, DC line = 1 means transfer command; vice versa */
        unsigned int sio_mode: 1;        /*!< Read and write through a single data line (MOSI) */
        unsigned int lsb_first: 1;       /*!< transmit LSB bit first if lsb_first was set to 1*/
        unsigned int cs_high_active: 1;  /*!< CS line is high active if cs_high_active was set to 1*/
    } flags; /*!< Extra flags to fine-tune the SPI device */
} epd_panel_io_spi_config_t;

/**
 * @brief Create e-paper panel IO handle, for SPI interface
 *
 * @param[in] bus SPI bus handle
 * @param[in] io_config IO configuration, for SPI interface
 * @param[out] ret_io Returned IO handle
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_ERR_NO_MEM        if out of memory
 *          - ESP_OK                on success
 */
esp_err_t epd_new_panel_io_spi(spi_host_device_t bus, const epd_panel_io_spi_config_t *io_config, epd_panel_io_handle_t *ret_io);


#ifdef __cplusplus
}
#endif
