cmake_minimum_required(VERSION 3.8)
project(Assignment2)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(Assignment2 ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} pthread)
set( CMAKE_CXX_FLAGS  "<other compiler flags> -pthread")