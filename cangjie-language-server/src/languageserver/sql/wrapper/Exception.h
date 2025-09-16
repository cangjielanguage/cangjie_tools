// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "SQLiteAPI.h"

#include <stdexcept>
#include <string>

namespace sqldb {

/**
 * Exception class for SQLite-specific errors.
 */
class Exception : public std::runtime_error {
public:
    Exception(int RC, std::string Msg);
    Exception(sqlite3 *DB, std::string Msg);
};

} // namespace sqldb