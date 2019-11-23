cmake_minimum_required(VERSION 3.13.4)
project(server)

set(CMAKE_C_FLAGS "-Wall -g -std=gnu11")

#add_definitions(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING)

add_executable(${PROJECT_NAME} server.cpp json_helper.hpp)

target_link_libraries(${PROJECT_NAME} pthread boost_thread boost_system)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/server.json
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include(CTest)
enable_testing()
