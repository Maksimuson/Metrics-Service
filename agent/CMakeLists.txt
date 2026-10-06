cmake_minimum_required(VERSION 3.18)
project(agent CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_library(HTTPLIB_LIB NAMES cpp-httplib httplib)

add_executable(agent src/main.cpp src/collector.cpp src/sender.cpp)
target_link_libraries(agent PRIVATE Threads::Threads)

if(HTTPLIB_LIB)
  target_link_libraries(agent PRIVATE ${HTTPLIB_LIB})
endif()