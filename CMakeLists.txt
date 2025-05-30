cmake_minimum_required(VERSION 3.10)  # 建议升级到较新版本
project(OS_ZWKL)

# 设置C++标准
set (CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 可执行文件输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 包含头文件目录（合并重复的include_directories）
include_directories(
    src/include
    thirdparty
    ${CMAKE_SOURCE_DIR}/thirdparty/SDL  # SDL2头文件路径
)

# # 指定SDL2动态库路径
# set(SDL2_LIBRARY ${CMAKE_SOURCE_DIR}/thirdparty/SDL/libSDL2.so)
# set(SDL2_IMAGE_LIBRARY ${CMAKE_SOURCE_DIR}/thirdparty/SDL/libSDL2_image.so)
# set(SDL2_TTF_LIBRARY ${CMAKE_SOURCE_DIR}/thirdparty/SDL/libSDL2_ttf.so)


# # 设置 RPATH（运行时库搜索路径）
# set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
# set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../thirdparty/SDL")

# 设置子目录
add_subdirectory(src/disk_formatting)

# 主程序源码
set(MAIN_SOURCES
    src/main.cpp
    src/just_test.cpp
    src/block/superBlock.cpp
    src/block/ddq.cpp
    src/dentry/dentry.cpp
    # src/parse/lex.yy.cc
    src/shell/file_system_manager.cpp
    src/file/filemanager.cpp
    src/shell/tool.cpp
)

# 创建主程序可执行文件（必须在target_link_libraries之前）
add_executable(main ${MAIN_SOURCES})

# 链接库（添加SDL2依赖的系统库）
target_link_libraries(main)
    # PRIVATEs
    # ${SDL2_LIBRARY}
    # ${SDL2_IMAGE_LIBRARY}
    # ${SDL2_TTF_LIBRARY}
    # pthread
    # dl       # SDL2可能需要
    # m        # 数学库
