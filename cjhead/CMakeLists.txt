# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.14.1)
project(cjhead)

if (MSVC AND ${CMAKE_VERSION} VERSION_GREATER "3.14")
    cmake_policy(SET CMP0091 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CJHEAD_VERSION_STR "0.0.1")
set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
set(FUZZ_FLAGS "-fsanitize=address -fsanitize-coverage=trace-pc-guard")
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")

if(WIN32)
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
endif()
string(TOLOWER ${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_cjnative output_lib_dir)


if (NOT APPLE AND NOT WIN32 AND NOT CMAKE_BUILD_TYPE MATCHES Debug AND NOT COVERAGE_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -pie -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined")
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
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE AND
        NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(COVERAGE_FLAG "Do coverage" OFF)
option(CANGJIE_AD_FLAG "Compile autodiff" OFF)
option(CANGJIE_CHIR_AD_FLAG "Compile chir autodiff" OFF)
option(CJLINT_JET_BACKEND "Build a version for JET backend" OFF)

add_compile_definitions(CANGJIE_AST2CHIR2)

if (CJLINT_JET_BACKEND)
    add_definitions(-DCJLINT_JET_BACKEND)
    add_compile_definitions(CANGJIE_CODEGEN_CJVM_BACKEND)
    set(CANGJIE_AD_FLAG OFF)
else()
    add_compile_definitions(CANGJIE_CODEGEN_CJNATIVE_BACKEND)
endif ()

if (CANGJIE_AD_FLAG)
    add_definitions(-DCANGJIE_AD)
endif()

if (CANGJIE_CHIR_AD_FLAG)
    add_definitions(-DCANGJIE_CHIR_AD)
endif()

if(GCC_TOOLCHAIN_FLAG)
    add_definitions(--gcc-toolchain="${GCC_TOOLCHAIN_FLAG}")
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

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
set(CANGJIE_ROOT ${THIRD_PARTY_DIR}/cangjie)
set(CANGJIE_OUTPUT ${CANGJIE_ROOT}/build)
set(CJC_DIR ${CANGJIE_ROOT}/bin)
set(JSON_ROOT ${THIRD_PARTY_DIR}/json-v3.11.3)

find_library(SECUREC_LIB
    NAMES libsecurec.so libsecurec.dylib libsecurec.dll
    PATHS
        ${CANGJIE_ROOT}/build/build/lib
        ${CANGJIE_ROOT}/output/lib/linux_aarch64_jet
        ${CANGJIE_ROOT}/output/lib/linux_x86_64_jet
)
if(NOT SECUREC_LIB)
    set(SECUREC_LIB securec)
endif()

set(LIB_FFI ffi)

LIST(APPEND labList
        pthread
        ${LIB_SSP}
        ${LIB_DL}
        ${LIB_FFI}
        ${SECUREC_LIB}
        )

if(COVERAGE_FLAG)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

if (CJLINT_ENABLE_ASAN)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ASAN_FLAGS}")
endif ()

if (CJLINT_ENABLE_FUZZ)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FUZZ_FLAGS}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FUZZ_FLAGS}")
endif ()

if (NOT EXISTS ${CANGJIE_OUTPUT})
    execute_process(COMMAND bash -c "./build.py build --no-js --no-tests  --llvmgc -t ${CMAKE_BUILD_TYPE}" WORKING_DIRECTORY "${CANGJIE_ROOT}")
endif ()

add_compile_definitions(CJHEAD_VERSION="${CJHEAD_VERSION_STR}")
message(STATUS "cjhead version: ${CJHEAD_VERSION_STR}")

message(STATUS "Obtain cjc version")
execute_process(COMMAND bash -c "./cjc -v" OUTPUT_VARIABLE CJC_VERSION ERROR_VARIABLE ERROR_MSG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY "${CJC_DIR}")
if (ERROR_MSG)
    message(STATUS "Obtain cjc version failed in directory ${CJC_DIR}: ${ERROR_MSG}")
else()
    message(STATUS "${CJC_VERSION}")
    add_compile_definitions(CJC_VERSION="${CJC_VERSION}")
endif ()

if (BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()
if (CHAR_VERSION)
    add_compile_definitions(VERSION_TAIL="${CHAR_VERSION}")
endif ()

add_definitions(-DCANGJIE_PROJECT_BUILD_DIR=\"${CANGJIE_ROOT}/build/build\")

set(JSON_HEADERS ${JSON_ROOT}/include)
if(CJLINT_JET_BACKEND)
    set(CANGJIE_HEADERS ${CANGJIE_ROOT}/includeVM)
else()
    set(CANGJIE_HEADERS ${CANGJIE_ROOT}/include)
endif()
set(HUAWEI_SECURE_C_HEADERS ${CANGJIE_ROOT}/third_party/huawei_secure_c/include)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/include)
include_directories(${JSON_HEADERS})
include_directories(${CANGJIE_HEADERS})
if(CJLINT_JET_BACKEND)
include_directories(AFTER ${CANGJIE_ROOT}/include)
endif()
include_directories(${HUAWEI_SECURE_C_HEADERS})

if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Debug)
    set(WIN_DEBUG_USE_STATIC_LIB ON)
endif()
if(WIN_DEBUG_USE_STATIC_LIB)
    add_library(cangjie-lsp STATIC IMPORTED)
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_LOCATION ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.dll.a
    )
else()
    add_library(cangjie-lsp SHARED IMPORTED)
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_LOCATION ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.so
    )
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_IMPLIB ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.dll.a
    )
endif()

if(APPLE)
    set_target_properties(cangjie-lsp PROPERTIES
    IMPORTED_LOCATION ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.dylib
    )
endif()

add_subdirectory(src)
