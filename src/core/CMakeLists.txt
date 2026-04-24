
add_library(${CORE_LIB} OBJECT "src/core.c")
target_include_directories(${CORE_LIB} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(${CORE_LIB} PRIVATE ${IO_LIB})