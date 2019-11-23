cmake_minimum_required(VERSION 3.13.4)
project(client)

set(CMAKE_C_FLAGS "-Wall -g -std=gnu11")

add_executable(${PROJECT_NAME} client.cpp)

target_link_libraries(${PROJECT_NAME} pthread boost_system unit_test_framework)

#find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

#target_link_libraries(testTheTester
#                      ${Boost_FILESYSTEM_LIBRARY}
#                      ${Boost_SYSTEM_LIBRARY}
#                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
#                      )
include(CTest)
enable_testing()
