set(MODULE_NAME reactor)
aux_source_directory(. MODULE_SRC)
add_library(${MODULE_NAME} ${MODULE_SRC})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_include_directories(${MODULE_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src/log ${PROJECT_SOURCE_DIR}/src/service)
target_link_libraries(${MODULE_NAME} PUBLIC Threads::Threads mysqlclient log service)