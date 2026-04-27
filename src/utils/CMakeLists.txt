
add_library(${UTILS_LIB} OBJECT "src/utils.c")
target_include_directories(${UTILS_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(${IO_LIB} OBJECT "src/io.c")
target_include_directories(${IO_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(${LOGGING_LIB} OBJECT "src/logging.c")
target_compile_definitions(${LOGGING_LIB} PRIVATE LOG_CONFIG_FILE="${LOG_CONFIG_FILE}")
target_include_directories(${LOGGING_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(${LOGGING_LIB} PUBLIC ${ZLOG_LIB})