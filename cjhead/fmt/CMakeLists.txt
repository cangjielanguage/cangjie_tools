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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
set(FUZZ_FLAGS "-fsanitize=address -fsanitize-coverage=trace-pc-guard")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")

option(CANGJIEFORMAT_BUILD_TESTS "Build cjfmt tests" ON)
option(CJFMT_CODE_COVERAGE "Do cjfmt code coverage" OFF)
option(CJFMT_ENABLE_ASAN "Build with asan" OFF)
option(CJFMT_ENABLE_FUZZ "Build with fuzz" OFF)
option(CANGJIE_AD_FLAG "Compile autodiff" OFF)
option(CJFMT_JET_BACKEND "Build a version for JET backend" OFF)

if (NOT APPLE AND NOT WIN32 AND NOT CMAKE_BUILD_TYPE MATCHES Debug AND NOT CJFMT_CODE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -pie -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -pie -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined")
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
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -pie")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -pie")
    SET(CMAKE_SKIP_RPATH TRUE)
    SET(CMAKE_SKIP_BUILD_RPATH TRUE)
endif ()

if (CJFMT_CODE_COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
endif ()

if (CJFMT_ENABLE_ASAN)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ASAN_FLAGS}")
endif ()

if (CJFMT_ENABLE_FUZZ)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FUZZ_FLAGS}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FUZZ_FLAGS}")
endif ()

if (NOT CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
    set(CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG /experimental:external /external:W0)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /external:I)
endif ()

if(GCC_TOOLCHAIN_FLAG)
    add_definitions(--gcc-toolchain="${GCC_TOOLCHAIN_FLAG}")
endif()

foreach(libpath ${CJFMT_TARGET_LIB})
    add_link_options("-L${libpath}")
endforeach()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
string(TOLOWER "${CMAKE_BUILD_TYPE}" lowercase_CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE AND
        NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif ()

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/build/bin)
install(PROGRAMS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cjfmt DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/config/cangjie-format.toml DESTINATION config)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNDEBUG)
endif ()
if(CJFMT_JET_BACKEND)
    add_compile_definitions(CANGJIE_CODEGEN_CJVM_BACKEND)
    add_definitions(-DJET_BACKEND)
    set(CANGJIE_AD_FLAG OFF)
endif()

if (CANGJIE_AD_FLAG)
    add_definitions(-DCANGJIE_AD)
endif()

add_compile_options(${CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG})

include_directories(${CMAKE_SOURCE_DIR}/third_party/cangjie/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

if (WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set(LIB_SEC ssp)
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

if (NOT EXISTS ${CANGJIE_OUTPUT})
    message(STATUS "Begin build cangjie")
    if (WIN32)
        execute_process(COMMAND python build.py build --no-js --no-tests -t ${lowercase_CMAKE_BUILD_TYPE} WORKING_DIRECTORY "${CANGJIE_ROOT}")
    else ()
        execute_process(COMMAND bash -c "./build.py build --llvmgc --no-js --no-tests -t ${lowercase_CMAKE_BUILD_TYPE}" WORKING_DIRECTORY "${CANGJIE_ROOT} --no-interp")
    endif ()
endif ()
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_library(cangjie STATIC IMPORTED)
    set_target_properties(cangjie PROPERTIES
            IMPORTED_LOCATION ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.a
            )
else()
    add_library(cangjie SHARED IMPORTED)
    set_target_properties(cangjie PROPERTIES
            IMPORTED_LOCATION ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.so
            )
    set_target_properties(cangjie PROPERTIES
            IMPORTED_IMPLIB ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.dll.a
            )
endif()


add_subdirectory(src)
if(APPLE)
    set_target_properties(cangjie PROPERTIES
    IMPORTED_LOCATION ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.dylib
    )
endif()
