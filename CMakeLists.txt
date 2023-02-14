cmake_minimum_required(VERSION 3.12.4)  # 必须

#设置版本信息
set(PRJ_VERSION_MAJOR 0)
set(PRJ_VERSION_MINOR 0)
set(PRJ_VERSION_PATCH 1)

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

set (PRJ_BUILD_SRCS)

project(test_lvgl C ASM)  #工程名

configure_file (
  "./cmake/config.h.in"
  "./cmake/config.h"
  )

include_directories("./sys/include") # 头文件目录
include_directories("${CMAKE_BINARY_DIR}/cmake")
include_directories("./src")
include_directories("./thirdparty/lvgl")
include_directories("./thirdparty/lvgl/indev")
include_directories("./thirdparty/lvgl/display")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2") # 设置C工程的 CFLAGS
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Werror") # 设置C++ 工程的 CXX_FLAGS
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -x assembler-with-cpp")

set(EXECUTABLE_OUTPUT_PATH bin)

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

#=====================================================================================================
# 下面使用install作为项目打包使用
set(CMAKE_INSTALL_PREFIX dist)  # 自定义安装目录，打包使用
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin) # 打包二进制文件

#set(TOOLS ${PRJ_ROOT_DIR}/tools/.cf) 
#install(FILES ${TOOLS} DESTINATION tools)
#install(FILES ${PRJ_ROOT_DIR}/script/a.sh DESTINATION tools)
install(FILES ${CMAKE_BINARY_DIR}/version DESTINATION ./)

# pack
#设置安装包的包名,默认为工程名
#set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/pack)
#set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${PRJ_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PRJ_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PRJ_VERSION_PATCH}")
set(PROJECT_VERSION_FULL ${PRJ_VERSION_MAJOR}.${PRJ_VERSION_MINOR}.${PRJ_VERSION_PATCH})

set(CPACK_GENERATOR "TGZ")
#set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION_FULL})
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION_FULL})
set(CPACK_PACKAGE_CHECKSUM "MD5")

include(CPack)
#MESSAGE(STATUS "CPACK_PACKAGE_INSTALL_DIRECTORY:${CPACK_OUTPUT_FILE_PREFIX}")

#=====================================================================================================
# 在目标构建完成后执行一些操作
add_custom_command(
  #生成版本号
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${PRJ_ROOT_DIR}/cmake/generate_version.sh ${CPACK_PACKAGE_FILE_NAME} ${CMAKE_BINARY_DIR}/version
  BYPRODUCTS ${CMAKE_BINARY_DIR}/version
  WORKING_DIRECTORY ${PRJ_ROOT_DIR}
  VERBATIM)
