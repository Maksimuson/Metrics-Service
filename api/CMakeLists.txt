cmake_minimum_required(VERSION 3.18)
project(api CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_library(HTTPLIB_LIB NAMES cpp-httplib httplib)
find_library(MYSQL_LIB NAMES mysqlclient REQUIRED)

add_executable(api src/main.cpp src/db.cpp)
target_link_libraries(api PRIVATE Threads::Threads ${MYSQL_LIB})

if(HTTPLIB_LIB)
  target_link_libraries(api PRIVATE ${HTTPLIB_LIB})
endif()