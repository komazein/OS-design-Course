cmake_minimum_required(VERSION 2.8.12)
project(OS_ZWKL)

# 可执行文件输出到项目根目录的bin文件夹
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)   

# 包含头文件目录
include_directories(src/include)

# 包含头文件目录
include_directories(thirdparty)


# 源文件
set(SOURCES
    src/main.cpp
    src/just_test.cpp
    src/block/superBlock.cpp            # if new *.cpp file add to the project, don't forget to add here
    src/block/ddq.cpp
    src/dentry/dentry.cpp
    src/shell/shellmanager.cpp
    src/shell/tool.cpp
)

# 创建可执行文件
add_executable(main ${SOURCES})