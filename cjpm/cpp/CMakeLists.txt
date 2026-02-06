#  Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
#  This source file is part of the Cangjie project, licensed under Apache-2.0
#  with Runtime Library Exception.
# 
#  See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(fswatcher 
    LANGUAGES C CXX
    DESCRIPTION "Cross-platform file watcher static library based on libuv")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror=return-type)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/third_party/libuv/CMakeLists.txt)
    message(FATAL_ERROR "libuv source not found! Please clone libuv to third_party/libuv first.")
endif()

add_subdirectory(third_party/libuv EXCLUDE_FROM_ALL)

file(GLOB_RECURSE FSWATCHER_SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_library(fswatcher STATIC
    ${FSWATCHER_SRC_FILES}
)

target_include_directories(fswatcher
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/third_party/libuv/include
)

target_link_libraries(fswatcher
    PRIVATE
        uv_a
)

set_target_properties(fswatcher PROPERTIES
    OUTPUT_NAME "fswatcher"
    ARCHIVE_OUTPUT_NAME "fswatcher"
    DEBUG_POSTFIX "d"
)

message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Implicit Link Directories: ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}")
message(STATUS "C++ Implicit Link Libraries: ${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES}")
message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS}")