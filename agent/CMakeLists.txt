cmake_minimum_required(VERSION 3.18)
project(agent CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build unit tests" OFF)

find_package(Threads REQUIRED)
find_library(HTTPLIB_LIB NAMES cpp-httplib httplib)

add_library(collector src/collector.cpp)
target_include_directories(collector PUBLIC src)

add_executable(agent src/main.cpp src/sender.cpp)
target_link_libraries(agent PRIVATE collector Threads::Threads)

if(HTTPLIB_LIB)
  target_link_libraries(agent PRIVATE ${HTTPLIB_LIB})
endif()

if(BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  add_executable(test_collector tests/test_collector.cpp)
  target_link_libraries(test_collector PRIVATE collector GTest::gtest_main)
  add_test(NAME collector COMMAND test_collector)
endif() 