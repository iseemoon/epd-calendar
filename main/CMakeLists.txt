file(GLOB_RECURSE COMMON_SOURCES common/*.c)
file(GLOB_RECURSE MISC_SOURCES misc/*.c)
file(GLOB_RECURSE UI_SOURCES ui_lvgl/*.c)
file(GLOB_RECURSE ASSETS_SOURCES assets/*.c )
file(GLOB_RECURSE IMAGE_SOURCES http/*.c)
file(GLOB_RECURSE CLOUD_SOURCES cloud/*.c)
file(GLOB_RECURSE DRIVERS_SOURCES drivers/*.c )

idf_component_register(SRCS 
            "app_main.c" 
            ${IMAGE_SOURCES} 
            ${COMMON_SOURCES} 
            ${MISC_SOURCES} 
            ${CLOUD_SOURCES}
            ${UI_SOURCES}
            ${ASSETS_SOURCES}
            ${DRIVERS_SOURCES}
        INCLUDE_DIRS "." "${EXTRA_COMPONENT_DIRS}/include"
        REQUIRES freetype lvgl sht30 json epd littlefs
            esp_event 
            esp_netif 
            esp_adc 
            esp_wifi 
            esp_http_client
            esp_https_server
            nvs_flash
            spi_flash
            mbedtls
        )
