cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/cxx/experimental/experimental_cpp_component")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(i80_controller)

littlefs_create_partition_image(fs flash_data)
# littlefs_create_partition_image(fs flash_data FLASH_IN_PROJECT)