cmake_minimum_required(VERSION 3.10)  # 建议升级到较新版本
project(OS_ZWKL)




# 设置C++标准
# set (CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 可执行文件输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 包含头文件目录（合并重复的include_directories）
include_directories(
    src/include
    thirdparty
)
# 设置C++标准
#set (CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 可执行文件输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 包含头文件目录（合并重复的include_directories）
include_directories(
    src/include
    thirdparty
)


# 手动指定SDL2库路径（根据实际路径修改）
set(SDL2_PATH "/usr/include/SDL2")          # SDL2头文件目录
set(SDL2_LIB "/usr/lib/x86_64-linux-gnu")   # SDL2库文件目录

# 手动指定SDL2_image路径
set(SDL2_IMAGE_PATH "/usr/include/SDL2")
set(SDL2_IMAGE_LIB "/usr/lib/x86_64-linux-gnu")

# 手动指定SDL2_ttf路径
set(SDL2_TTF_PATH "/usr/include/SDL2")
set(SDL2_TTF_LIB "/usr/lib/x86_64-linux-gnu")

# 查找头文件
find_path(SDL2_INCLUDE_DIR SDL.h PATHS ${SDL2_PATH} REQUIRED)
find_path(SDL2_IMAGE_INCLUDE_DIR SDL_image.h PATHS ${SDL2_IMAGE_PATH} REQUIRED)
find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h PATHS ${SDL2_TTF_PATH} REQUIRED)

# 查找库文件
find_library(SDL2_LIBRARY SDL2 PATHS ${SDL2_LIB} REQUIRED)
find_library(SDL2_IMAGE_LIBRARY SDL2_image PATHS ${SDL2_IMAGE_LIB} REQUIRED)
find_library(SDL2_TTF_LIBRARY SDL2_ttf PATHS ${SDL2_TTF_LIB} REQUIRED)


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
    src/front/front.cpp
    src/block/superBlock.cpp
    src/block/ddq.cpp
    src/dentry/dentry.cpp
    src/parse/lex.yy.cc
    src/shell/file_system_manager.cpp
    src/file/filemanager.cpp
    src/shell/tool.cpp
    src/log_global.cpp
    src/user/user.cpp
    # src/MSG/MSG.cpp
)

# 创建主程序可执行文件（必须在target_link_libraries之前）
add_executable(main ${MAIN_SOURCES})

# 链接库（添加SDL2依赖的系统库）
#target_link_libraries(main)
    # PRIVATEs
    # ${SDL2_LIBRARY}
    # ${SDL2_IMAGE_LIBRARY}
    # ${SDL2_TTF_LIBRARY}
    # pthread
    # dl       # SDL2可能需要
    # m        # 数学库
# 链接库（添加SDL2依赖的系统库）
target_link_libraries(main
    PRIVATE
    ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_TTF_LIBRARY}
    pthread
    dl       # SDL2可能需要
    m        # 数学库
)