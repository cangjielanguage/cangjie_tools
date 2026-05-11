# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

include_directories(${CMAKE_SOURCE_DIR}/third_party/sqlite3)

add_library(Analyzer OBJECT
    HeapAnalyzer.cpp
    HttpHandlers.cpp
    HttpServer.cpp
    Logger.cpp
    SqliteDb.cpp
    DatabaseSchema.cpp
)
set_target_properties(Analyzer PROPERTIES POSITION_INDEPENDENT_CODE ON)
