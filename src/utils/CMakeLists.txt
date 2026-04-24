
add_library(${UTILS_LIB} OBJECT "src/utils.c")
target_include_directories(${UTILS_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(${IO_LIB} OBJECT "src/io.c")
target_include_directories(${IO_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")