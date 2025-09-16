// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Column.h"
#include "BlobView.h"

#include "sqlite3.h"

namespace sqldb {

Column::Column(sqlite3_stmt *Stmt, int Index) noexcept : Stmt(Stmt), Index(Index) {}

std::string_view Column::getName() const noexcept { return sqlite3_column_name(Stmt, Index); }

BlobView Column::getBlob() const noexcept
{
    return {static_cast<const void *>(sqlite3_column_blob(Stmt, Index)),
        static_cast<std::size_t>(sqlite3_column_bytes(Stmt, Index))};
}

std::string_view Column::getText() const noexcept
{
    return {static_cast<const char *>(sqlite3_column_blob(Stmt, Index)),
        static_cast<std::size_t>(sqlite3_column_bytes(Stmt, Index))};
}

std::int64_t Column::getInt64() const noexcept { return sqlite3_column_int64(Stmt, Index); }

double Column::getDouble() const noexcept { return sqlite3_column_double(Stmt, Index); }

bool Column::isBlob() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_BLOB; }

bool Column::isFloat() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_FLOAT; }

bool Column::isInteger() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_INTEGER; }

bool Column::isText() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_TEXT; }

bool Column::isNull() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_NULL; }

} // namespace sqldb