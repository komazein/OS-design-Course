cmake_minimum_required(VERSION 2.8.12)
project(disk_formatting)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/src/include)

# 源文件
set(SOURCES
    disk_formatting.cpp
    ${CMAKE_SOURCE_DIR}/src/block/superBlock.cpp
    ${CMAKE_SOURCE_DIR}/src/block/ddq.cpp
    ${CMAKE_SOURCE_DIR}/src/dentry/dentry.cpp
)

# 创建格式化工具可执行文件
add_executable(dft ${SOURCES})
