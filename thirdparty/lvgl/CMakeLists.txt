cmake_minimum_required(VERSION 3.12.4)

project(lvgl C)

set(LVGL_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE SUB_SRCS ${LVGL_ROOT_DIR}/src/*.c)
list(APPEND SUB_SRCS ${LVGL_ROOT_DIR}/indev/evdev.c ${LVGL_ROOT_DIR}/display/fbdev.c)
file(GLOB LV_DRIVERS_PUBLIC_HEADERS 
	"${LVGL_ROOT_DIR}/lvgl.h")


add_library(lvgl STATIC ${SUB_SRCS})

set_target_properties(
  lvgl
  PROPERTIES OUTPUT_NAME lvgl
             ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             PUBLIC_HEADER "${LV_DRIVERS_PUBLIC_HEADERS}")

target_include_directories(lvgl PRIVATE ${LVGL_ROOT_DIR})
