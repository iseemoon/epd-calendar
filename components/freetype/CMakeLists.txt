set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFT2_BUILD_LIBRARY")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFT_CONFIG_MODULES_H=<lvgl/src/libs/freetype/ftmodule.h>")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFT_CONFIG_MODULES_H=<lvgl/src/libs/freetype/ftoption.h>")

idf_component_register(SRCS 
        "lv_freetype.c"
        "freetype/src/base/ftbase.c"
        "freetype/src/base/ftbitmap.c"
        "freetype/src/base/ftdebug.c"
        "freetype/src/base/ftglyph.c"
        "freetype/src/base/ftinit.c"
        "freetype/src/base/ftsystem.c"
        "freetype/src/cache/ftcache.c"
        "freetype/src/gzip/ftgzip.c"
        "freetype/src/sfnt/sfnt.c"
        "freetype/src/smooth/smooth.c"
        "freetype/src/truetype/truetype.c"
        INCLUDE_DIRS "." "freetype/include" 
        REQUIRES lvgl)
