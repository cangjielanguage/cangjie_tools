# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

if(CMAKE_BACKEND STREQUAL "cjnative")
    add_library(Recorder OBJECT
        LinuxRecorder.cpp
    )
else()
  message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()