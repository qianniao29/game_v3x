cmake_minimum_required(VERSION 3.12.4)  # 必须

set(PRJ_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_C_STANDARD 11)	# C11
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CONFIG_PREFIX "/home/ymc/bin/gcc-linaro-11.0.1-2021.03-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-")
set(CMAKE_C_COMPILER "${CONFIG_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${CONFIG_PREFIX}g++")
set(CMAKE_ASM_COMPILER "${CONFIG_PREFIX}gcc")
set(CMAKE_OBJCOPY "${CONFIG_PREFIX}objcopy")
set(CMAKE_C_AR "${CONFIG_PREFIX}ar")
set(CMAKE_SIZE "${CONFIG_PREFIX}size")

#Disable compiler checks
#set(CMAKE_C_COMPILER_FORCED TRUE)
#set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(LD_LIBS m pthread lvgl evdev mtdev udev input drm asound avformat avcodec avutil swscale z ssl crypto swresample)

project(test_lvgl C ASM)  #工程名

configure_file (
  "./cmake/config.h.in"
  "./cmake/config.h"
  )

set (PRJ_VERSION_MAJOR 1)
set (PRJ_VERSION_MINOR 0)
set (PRJ_BUILD_SRCS)

include_directories("./sys/include") # 头文件目录
include_directories("${CMAKE_BINARY_DIR}/cmake")
include_directories("./src")
include_directories("./thirdparty/lvgl")
include_directories("./thirdparty/lvgl/indev")
include_directories("./thirdparty/lvgl/display")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2") # 设置C工程的 CFLAGS
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Werror") # 设置C++ 工程的 CXX_FLAGS
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -x assembler-with-cpp")

add_definitions(
    "-DLV_CONF_INCLUDE_SIMPLE"
    "-DLV_LVGL_H_INCLUDE_SIMPLE"
)
include (${PRJ_ROOT_DIR}/src/CMakeLists.txt)
include (${PRJ_ROOT_DIR}/thirdparty/CMakeLists.txt)
# MESSAGE("BUILD_SRCS: ${PRJ_BUILD_SRCS}")

link_directories("./sys/lib") # 链接库目录

add_executable(${PROJECT_NAME} ${PRJ_BUILD_SRCS} ${PRJ_ASM_SRCS}) # 生成可执行文件，这里程序名即为功能名

target_link_libraries(${PROJECT_NAME} ${LD_LIBS}) # 链接库

# 下面使用install作为项目打包使用
set(CMAKE_INSTALL_PREFIX dist)  # 自定义安装目录，打包使用
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin) # 打包二进制文件

#set(CONFIGS ${PROJECT_SOURCE_DIR}/hello.cf) 
#install(FILES ${CONFIGS} DESTINATION config) # 打包配置文件

