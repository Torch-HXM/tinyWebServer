cmake_minimum_required (VERSION 3.0)

project (server)

# add var
add_definitions(-DPROJECT_PATH=\"${PROJECT_SOURCE_DIR}/\")
add_definitions(-DLOGGER=\"ON\")

add_subdirectory(src/log)
add_subdirectory(src/service)
add_subdirectory(src/reactor)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC src/log src/reactor)

target_link_libraries(${PROJECT_NAME} PUBLIC log reactor)