# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

if(NOT CMAKE_SYSROOT)
    message(FATAL_ERROR "Please specify CMAKE_SYSROOT")
endif()

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(TRIPLE "x86_64-w64-mingw32")
set(CANGJIE_TRIPLE "x86_64-windows-gnu")
set(CMAKE_C_COMPILER ${CMAKE_SYSROOT}/bin/${TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${CMAKE_SYSROOT}/bin/${TRIPLE}-g++)
