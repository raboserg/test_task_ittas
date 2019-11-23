cmake_minimum_required(VERSION 3.13.4)
project(client)

set(CMAKE_C_FLAGS "-Wall -g -std=gnu11")

add_executable(${PROJECT_NAME} client.cpp)

set (Boost_USE_STATIC_LIBS OFF)
find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} pthread boost_system)

include(CTest)
enable_testing()
