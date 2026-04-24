cmake_minimum_required(VERSION 3.10)

project(mats-lt-sensor-node C)

include("config.cmake")
include("${PROJECT_SOURCE_DIR}/profiles/${PROFILE_NAME}/config.cmake")

set(AHOI_LIB "ahoi-lib")
set(CORE_LIB "ahoi-core")
set(SCHC_LIB "schc")
set(SENSING_LIB "sensing")
set(STAT_LIB "stat")
set(UTILS_LIB "utils")
set(IO_LIB "io")

add_subdirectory(src)