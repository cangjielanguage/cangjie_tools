#  Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
#  This source file is part of the Cangjie project, licensed under Apache-2.0
#  with Runtime Library Exception.
# 
#  See https://cangjie-lang.cn/pages/LICENSE for license information.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

set(LLVM_MINGW_ROOT "/opt/buildtools/llvm-mingw-w64")
set(CMAKE_C_COMPILER "${LLVM_MINGW_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_MINGW_ROOT}/bin/clang++")
set(CMAKE_C_COMPILER_TARGET ${TOOLCHAIN_PREFIX})
set(CMAKE_CXX_COMPILER_TARGET ${TOOLCHAIN_PREFIX})

set(CMAKE_C_FLAGS_INIT "-target x86_64-w64-mingw32 --rtlib=compiler-rt")
set(CMAKE_CXX_FLAGS_INIT "-target x86_64-w64-mingw32 --rtlib=compiler-rt")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

set(CMAKE_SYSROOT "${LLVM_MINGW_ROOT}/x86_64-w64-mingw32")

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "强制编译静态库" FORCE)