cmake_minimum_required(VERSION 2.8.12) 
project(OS_ZWKL)

# 可执行文件输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)   

# 包含头文件目录
include_directories(src/include)
include_directories(thirdparty)

# 设置子目录
add_subdirectory(src/disk_formatting)

# 主程序源码
set(MAIN_SOURCES
    src/main.cpp
    src/just_test.cpp
    src/block/superBlock.cpp
    src/block/ddq.cpp
    src/dentry/dentry.cpp
    src/shell/file_system_manager.cpp
    # src/parse/lex.yy.cc
    src/shell/tool.cpp
)

# 创建主程序可执行文件
add_executable(main ${MAIN_SOURCES})
