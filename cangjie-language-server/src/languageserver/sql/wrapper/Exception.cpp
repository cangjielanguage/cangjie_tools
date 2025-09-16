// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Exception.h"

#include "sqlite3.h"

namespace sqldb {

Exception::Exception(int RC, std::string Msg) : std::runtime_error(Msg.append(": ").append(sqlite3_errstr(RC))) {}

Exception::Exception(sqlite3 *DB, std::string Msg) : std::runtime_error(Msg.append(": ").append(sqlite3_errmsg(DB))) {}

} // namespace sqldb