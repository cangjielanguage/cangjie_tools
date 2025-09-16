# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

file(GLOB_RECURSE JSON_RPC_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

message(STATUS "Found source files:")
foreach(file ${JSON_RPC_SRC})
    message(STATUS "${file}")
endforeach()

add_library(JsonRPC OBJECT ${JSON_RPC_SRC})