cmake_minimum_required(VERSION 3.16)
project(api CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_library(HTTPLIB_LIB NAMES cpp-httplib httplib)

add_executable(api src/main.cpp)
target_link_libraries(api PRIVATE Threads::Threads)

if(HTTPLIB_LIB)
  target_link_libraries(api PRIVATE ${HTTPLIB_LIB})
endif()