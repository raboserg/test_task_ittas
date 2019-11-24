cmake_minimum_required(VERSION 3.13.4)
project(client)

set(CMAKE_C_FLAGS "-Wall -g -std=gnu11")

add_executable(${PROJECT_NAME} client.cpp)

target_link_libraries(${PROJECT_NAME} pthread boost_system)
