# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.14.1)
project(cangjieComplienceChecker)

if (MSVC AND ${CMAKE_VERSION} VERSION_GREATER "3.14")
    cmake_policy(SET CMP0091 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CJCOMPAT_VERSION_STR "1.2.0-beta.rc2")
set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fstandalone-debug")

if (CANGJIE_HOME)
    message(STATUS "CANGJIE_HOME (from -D): ${CANGJIE_HOME}")
elseif (DEFINED ENV{CANGJIE_HOME})
    set(CANGJIE_HOME $ENV{CANGJIE_HOME})
    message(STATUS "CANGJIE_HOME: ${CANGJIE_HOME}")
else()
    message("Please specify the path to the Cangjie SDK via the environment variable CANGJIE_HOME or by using the -DCANGJIE_HOME option.")
endif()

if(WIN32)
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
endif()
string(TOLOWER ${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_cjnative output_lib_dir)


if (NOT APPLE AND NOT WIN32 AND NOT CMAKE_BUILD_TYPE MATCHES Debug AND NOT COVERAGE_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -Wl,-z,relro,-z,now,-z,noexecstack")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -pie -Wl,-z,relro,-z,now,-z,noexecstack")
    SET(CMAKE_SKIP_RPATH TRUE)
    SET(CMAKE_SKIP_BUILD_RPATH TRUE)
endif ()

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-insert-timestamp")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-insert-timestamp")
endif()

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -pie")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -pie")
    SET(CMAKE_SKIP_RPATH TRUE)
    SET(CMAKE_SKIP_BUILD_RPATH TRUE)
endif ()

if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNDEBUG)
endif ()

if (CANGJIE_WIN_NATIVE_DEV)
    add_compile_definitions(CANGJIE_WIN_NATIVE_DEV)
    message(STATUS "Building for MinGW Windows (Developing)")
endif()


if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    add_definitions(-D__ARM__)
endif()

if(NOT CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
    set(CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG /experimental:external /external:W0)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /external:I)
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE AND
        NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(COVERAGE_FLAG "Do coverage" OFF)

add_definitions(-DCANGJIE_CODEGEN_CJNATIVE_BACKEND)

if(GCC_TOOLCHAIN_FLAG)
    add_compile_options(--gcc-toolchain=${GCC_TOOLCHAIN_FLAG})
    add_link_options(--gcc-toolchain=${GCC_TOOLCHAIN_FLAG})
endif()

foreach(libpath ${CANGJIE_TARGET_LIB})
    add_link_options("-L${libpath}")
endforeach()

add_compile_options(${CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG})

if(WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set(LIB_SEC ssp)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(NOT WIN32)
    set(LIB_DL dl)
endif()

if(CANGJIE_LOCAL)
    set(CANGJIE_ROOT ${CMAKE_SOURCE_DIR}/third_party/cangjie)
else()
    set(CANGJIE_ROOT ${CMAKE_SOURCE_DIR}/../../cangjie_compiler)
endif()
set(CANGJIE_OUTPUT ${CANGJIE_ROOT}/output)
set(CJC_DIR ${CANGJIE_OUTPUT}/bin)

LIST(APPEND labList
        pthread
        ${LIB_SSP}
        ${LIB_DL}
        )

if(COVERAGE_FLAG)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

if (CJCOMPAT_ENABLE_ASAN)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ASAN_FLAGS}")
endif ()

if (NOT EXISTS ${CANGJIE_OUTPUT})
    execute_process(COMMAND bash -c "./build.py build -t release" WORKING_DIRECTORY "${CANGJIE_ROOT}")
    execute_process(COMMAND bash -c "./build.py install" WORKING_DIRECTORY "${CANGJIE_ROOT}")
endif ()

add_compile_definitions(CJCOMPAT_VERSION="${CJCOMPAT_VERSION_STR}")
message(STATUS "Cangjie Compliance: ${CJCOMPAT_VERSION_STR}")

# Obtain CJC version
message(STATUS "Obtain cjc version")
execute_process(COMMAND bash -c "./cjc -v" OUTPUT_VARIABLE CJC_VERSION_STR ERROR_VARIABLE ERROR_MSG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY "${CJC_DIR}")
if (ERROR_MSG)
    message(STATUS "Obtain cjc version failed in directory ${CJC_DIR}: ${ERROR_MSG}")
else()
    message(STATUS "${CJC_VERSION_STR}") # CJC_VERSION_STR comes with "Cangjie version"
    string(REPLACE "\n" " " CJC_VERSION_CLEANED "${CJC_VERSION_STR}")
    add_compile_definitions(CJCOMPAT_CANGJIE_VER="${CJC_VERSION_CLEANED}")
endif ()

if (BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()
if (CHAR_VERSION)
    add_compile_definitions(VERSION_TAIL="${CHAR_VERSION}")
endif ()

#compiler option
add_definitions(-DCANGJIE_PROJECT_BUILD_DIR=\"${CANGJIE_ROOT}/build/build\")

set(CANGJIE_HEADERS ${CANGJIE_HOME}/include)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CANGJIE_HEADERS})

if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Debug)
    set(WIN_DEBUG_USE_STATIC_LIB ON)
endif()
if(WIN_DEBUG_USE_STATIC_LIB)
    add_library(cangjie-lsp STATIC IMPORTED)
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_LOCATION ${CANGJIE_HOME}/tools/lib/libcangjie-lsp.a
    )
else()
    add_library(cangjie-lsp SHARED IMPORTED)
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_LOCATION ${CANGJIE_HOME}/tools/lib/libcangjie-lsp.so
    )
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_IMPLIB ${CANGJIE_HOME}/tools/lib/libcangjie-lsp.dll.a
    )
endif()

if(APPLE)
    set_target_properties(cangjie-lsp PROPERTIES
    IMPORTED_LOCATION ${CANGJIE_HOME}/tools/lib/libcangjie-lsp.dylib
    )
endif()

add_subdirectory(src)
