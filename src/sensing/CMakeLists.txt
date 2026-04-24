
add_library(${SENSING_LIB} OBJECT "src/dummy_sensing.c")
target_include_directories(${SENSING_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(${STAT_LIB} OBJECT "src/stat.c")
target_include_directories(${STAT_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")