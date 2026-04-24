
add_library(${SCHC_LIB} OBJECT "src/schc_base.c" "src/schc_lu_table_commons.c" "src/tables/${SCHC_LU_TABLE}.c")
target_include_directories(${SCHC_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_compile_definitions(${SCHC_LIB} PRIVATE SENSOR_LINK_ID=${SENSOR_LINK_ID})
target_link_libraries(${SCHC_LIB} PRIVATE ${UTILS_LIB} ${SENSING_LIB})