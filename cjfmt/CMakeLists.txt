# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.14.1)
project(cjfmt)

if (MSVC AND ${CMAKE_VERSION} VERSION_GREATER "3.15")
    cmake_policy(SET CMP0091 NEW)
endif ()

set(CJFMT_VERSION_STR "1.0.0")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")

if (CANGJIE_HOME)
    message(STATUS "CANGJIE_HOME (from -D): ${CANGJIE_HOME}")
elseif (DEFINED ENV{CANGJIE_HOME})
    set(CANGJIE_HOME $ENV{CANGJIE_HOME})
    message(STATUS "CANGJIE_HOME: ${CANGJIE_HOME}")
else()
    message("Please specify the path to the Cangjie SDK via the environment variable CANGJIE_HOME or by using the -DCANGJIE_HOME option.")
endif()

option(CJFMT_CODE_COVERAGE "Do cjfmt code coverage" OFF)

if (NOT APPLE AND NOT WIN32 AND NOT CMAKE_BUILD_TYPE MATCHES Debug AND NOT CJFMT_CODE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined")
    SET(CMAKE_SKIP_RPATH TRUE)
    SET(CMAKE_SKIP_BUILD_RPATH TRUE)
endif ()

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-insert-timestamp")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-insert-timestamp")
endif()

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
    SET(CMAKE_SKIP_RPATH TRUE)
    SET(CMAKE_SKIP_BUILD_RPATH TRUE)
endif ()

if (CJFMT_CODE_COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    add_compile_definitions(CANGJIE_ENABLE_GCOV)
endif ()

if (NOT CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
    set(CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG /experimental:external /external:W0)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /external:I)
endif ()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
string(TOLOWER "${CMAKE_BUILD_TYPE}" lowercase_CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE AND
        NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif ()

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/build/bin)
install(FILES ${CMAKE_SOURCE_DIR}/config/cangjie-format.toml DESTINATION config)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNDEBUG)
endif ()

add_compile_options(${CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG})
add_compile_definitions(CJFMT_VERSION="${CJFMT_VERSION_STR}")

include_directories(${CANGJIE_HOME}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

if (WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set(LIB_SSP ssp)
else ()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif ()

if(NOT WIN32)
    set(LIB_DL dl)
endif()

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
set(CANGJIE_ROOT ${THIRD_PARTY_DIR}/cangjie)
set(CANGJIE_OUTPUT ${CANGJIE_ROOT}/build)

LIST(APPEND labList
        pthread
        ${LIB_SSP}
        ${LIB_DL}
        )

add_library(cangjie SHARED IMPORTED)
set_target_properties(cangjie PROPERTIES
        IMPORTED_LOCATION ${CANGJIE_HOME}/tools/lib/libcangjie-lsp.so
        )
set_target_properties(cangjie PROPERTIES
        IMPORTED_IMPLIB ${CANGJIE_HOME}/tools/lib/libcangjie-lsp.dll.a
        )

add_subdirectory(src)
if(APPLE)
    set_target_properties(cangjie PROPERTIES
    IMPORTED_LOCATION ${CANGJIE_HOME}/tools/lib/libcangjie-lsp.dylib
    )
endif()
install(TARGETS cjfmt RUNTIME DESTINATION bin)