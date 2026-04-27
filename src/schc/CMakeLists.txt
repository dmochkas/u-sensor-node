
add_library(${SCHC_LIB} OBJECT "src/schc_base.c" "src/schc_lu_table_commons.c" "src/tables/${SCHC_LU_TABLE}.c")
target_include_directories(${SCHC_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
if (N_INSTANCES_ENABLED)
    target_compile_definitions(${SCHC_LIB} PRIVATE N_INSTANCES_ENABLED MSB_SIZE=${MSB_SIZE} N_INSTANCES=${N_INSTANCES})
endif ()
target_link_libraries(${SCHC_LIB} PRIVATE ${UTILS_LIB} ${SENSING_LIB})