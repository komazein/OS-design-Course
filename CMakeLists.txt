cmake_minimum_required(VERSION 2.8.12)
project(zmqpp)

# 可执行文件输出到项目根目录的bin文件夹
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)   

add_executable(hello src/main.cpp src/just_test.cpp)