cmake_minimum_required(VERSION 3.10)

project(mats-lt-sensor-node C)

set(CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/profiles/"
    "${PROJECT_SOURCE_DIR}/cmake/"
)

include(config.cmake)
include(rule_gen_strategies)
include(${PROFILE_NAME}/config)

config_rule_gen_strategy("${SCHC_RULE_GEN_STRATEGY}")

set(ZLOG_LIB "zlog")

add_subdirectory(thirdparty/zlog)
target_include_directories(${ZLOG_LIB} PUBLIC "${PROJECT_SOURCE_DIR}/thirdparty/zlog/src")

set(AHOI_LIB "ahoi-lib")
set(CORE_LIB "ahoi-core")
set(SCHC_LIB "schc")
set(SENSING_LIB "sensing")
set(STAT_LIB "stat")
set(UTILS_LIB "utils")
set(IO_LIB "io")
set(LOGGING_LIB "logging")

add_subdirectory(src)