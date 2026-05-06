# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.10)
project(cjprof-jni)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(STRIP_LIB_PREFIX "Strip 'lib' prefix" OFF)

if (STRIP_LIB_PREFIX)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

set(JAVA_AWT_LIBRARY "NotNeeded")
set(JAVA_JVM_LIBRARY "NotNeeded")
set(JAVA_AWT_INCLUDE_PATH "NotNeeded")
find_package(JNI REQUIRED)
message(STATUS "Found JNI: ${JNI_INCLUDE_DIRS}")

include_directories(${JNI_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(../../include)

add_library(cjprof-jni SHARED
    cpp/CjprofJNI.cpp
)

if (CMAKE_BUILD_TYPE MATCHES Release AND MINGW)
    target_link_options(cjprof-jni PRIVATE -static -Wl,--allow-multiple-definition)
endif ()

set_target_properties(cjprof-jni PROPERTIES
    OUTPUT_NAME "cjprof-jni"
)

find_library(CJPROF_LIBRARY NAMES cjprof)
if(CJPROF_LIBRARY)
    target_link_libraries(cjprof-jni ${CJPROF_LIBRARY})
else()
    message(WARNING "cjprof library not found, please ensure it is in system library path")
endif()
