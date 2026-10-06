cmake_minimum_required(VERSION 3.16)
project(agent CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(agent src/main.cpp src/collector.cpp)
