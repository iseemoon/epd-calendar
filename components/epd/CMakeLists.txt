idf_component_register(SRCS "gdey075.c"
        "epd_panel_io_spi.c"
        "epd_panel_io.c"
        "epd_panel_ops.c"
    INCLUDE_DIRS "include"
    REQUIRES "driver" "lvgl")