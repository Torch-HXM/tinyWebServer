set(MODULE_NAME log)
aux_source_directory(. MODULE_SRC)
add_library(${MODULE_NAME} ${MODULE_SRC})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${MODULE_NAME} PUBLIC Threads::Threads)