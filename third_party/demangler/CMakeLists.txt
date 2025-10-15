# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.16.5)
project(cangjie-demangler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if(BUILD_GCC_TOOLCHAIN AND (CMAKE_C_COMPILER_ID STREQUAL "Clang" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")))
    add_compile_options(--gcc-toolchain=${BUILD_GCC_TOOLCHAIN})
    add_link_options(--gcc-toolchain=${BUILD_GCC_TOOLCHAIN})
endif()

if(CANGJIE_DEMANGLER_USE_CSTRING)
    # A library for demangling, which using CString as its string implementation.
    add_library(cangjie-demangle STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp)
else()
    # A library for demangling, which using std::string as its string implementation.
    add_library(cangjie-demangle STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CangjieDemangle.cpp)
    target_compile_definitions(cangjie-demangle PRIVATE BUILD_LIB_CANGJIE_DEMANGLE)
endif()
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CangjieDemangle.h DESTINATION include)
install(TARGETS cangjie-demangle DESTINATION lib)

if(OHOS)
    add_library(cangjie-demangle-shared SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CangjieDemangle.cpp)
    target_compile_definitions(cangjie-demangle-shared PRIVATE BUILD_LIB_CANGJIE_DEMANGLE)
    install(TARGETS cangjie-demangle-shared LIBRARY DESTINATION lib)
    set_target_properties(cangjie-demangle-shared PROPERTIES OUTPUT_NAME "cangjie-demangle")
endif()

# A command line tool for demangling Cangjie symbols.
add_executable(cjfilt
    ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Cjfilt.cpp)
target_compile_definitions(cjfilt PRIVATE BUILD_LIB_CANGJIE_DEMANGLE)
install(TARGETS cjfilt DESTINATION bin)
