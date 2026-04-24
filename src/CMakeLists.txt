add_subdirectory(utils)
add_subdirectory(core)

add_library(${AHOI_LIB} STATIC $<TARGET_OBJECTS:${CORE_LIB}>)
target_include_directories(${AHOI_LIB} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/core/include")

add_subdirectory(schc)
add_subdirectory(sensing)
add_subdirectory(app)