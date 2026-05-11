# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.10)

project("cjprof")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(STRIP_FLAG "-s")

option(STRIP_LIB_PREFIX "Strip 'lib' prefix" OFF)

if (STRIP_LIB_PREFIX)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

if (NOT APPLE AND NOT WIN32)
  set(LINK_FLAGS "-pie -rdynamic -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -Wl,--no-undefined")
endif ()

if (WIN32)
  set(LINK_FLAGS "-Wl,--no-insert-timestamp -Wl,--no-undefined")
endif ()

if (APPLE)
  set(LINK_FLAGS "-pie -rdynamic")
endif ()

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS} ${STRIP_FLAG}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINK_FLAGS} ${STRIP_FLAG}")
else()
    set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINK_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")

set(COV 0)
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_SKIP_BUILD_RPATH TRUE)

add_compile_definitions(CJNATIVE=1)

if(NOT CMAKE_BACKEND)
    set(CMAKE_BACKEND cjnative)
endif()

if(CMAKE_BACKEND STREQUAL "cjnative")
  message(STATUS "Configuring cjnatie build")
  add_compile_definitions(BACKEND=1)
else()
  message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

include_directories(third_party/boundscheck/include)
include_directories(third_party/runtime/runtime/src)
add_subdirectory(third_party/runtime/runtime/src/Demangler EXCLUDE_FROM_ALL)
set_target_properties(cangjie-demangle PROPERTIES EXCLUDE_FROM_ALL FALSE)

include_directories(include)
add_subdirectory(src)