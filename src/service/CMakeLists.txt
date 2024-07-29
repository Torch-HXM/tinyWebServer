set(MODULE_NAME service)
aux_source_directory(. MODULE_SRC)
add_library(${MODULE_NAME} ${MODULE_SRC})

target_include_directories(${MODULE_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src/log)
target_link_libraries(${MODULE_NAME} PUBLIC log)