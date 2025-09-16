# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.14.1)
project(CANGJIE_LSP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
if (CMAKE_BUILD_TYPE MATCHES "Release")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2 -O2")
endif ()

if (MINGW AND CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options("-Wa,-mbig-obj")
endif ()

if (APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif ()

# set default build type to debug
add_compile_definitions(NOT_RELEASE_VERSION_CODE)
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif ()
option(MACRO_DYNAMIC "Compile interpreter" ON)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()
if(ENABLE_TEST)
    add_definitions(-DTEST_FLAG)
endif()
if (MACRO_DYNAMIC)
    add_definitions(-DMACRO_DYNAMIC)
endif()
# Code coverage is disabled by default.
if(ENABLE_COVERAGE)
    message(STATUS "Use lcov for code coverage test")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

option(DEBUG_KERNEL "debug with Cangjie compiler kernel source code embeded" OFF)
link_directories(src/lib/cangjie/lib)
# set output path
set (CMAKE_INSTALL_PREFIX output)
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/lib)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
if(CROSS_WINDOWS)
 set(CMAKE_SYSTEM_NAME Windows)
 set(CMAKE_SYSTEM_PROCESSOR x86_64)
 set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
 set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
 set(CMAKE_LINKER x86_64-w64-mingw32-ld)
endif()
message("Directory of executable program: " ${EXECUTABLE_OUTPUT_PATH})
message("Directory of library: " ${LIBRARY_OUTPUT_PATH})
add_definitions(-DCANGJIE_CODEGEN_CJNATIVE_BACKEND)
# language server
add_subdirectory(src)
# gtest for unit test
if(ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()