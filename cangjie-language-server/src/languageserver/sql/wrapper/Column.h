// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "SQLiteAPI.h"
#include "BlobView.h"

#include <cstdint>
#include <string_view>

namespace sqldb {
/**
 * Non-owning view for a column in a row of data extracted by SQL query.
 */
class Column {
public:
    /**
     * Create a new column view object with the specified index for SQLite native
     * handle. The column view does not own or manager the lifetime of the
     * underlying SQLite native handle object.
     */
    Column(sqlite3_stmt *Stmt, int Index) noexcept;

    /**
     * Column objects are copy-constructible.
     */
    Column(const Column &) noexcept = default;

    /**
     * Column objects are not copy-assignable.
     */
    Column &operator=(const Column &) noexcept = delete;

    /**
     * Return the column name.
     */
    std::string_view getName() const noexcept;

    /**
     * Return the column value as BLOB.
     */
    BlobView getBlob() const noexcept;

    /**
     * Return the column value as TEXT.
     */
    std::string_view getText() const noexcept;

    /**
     * Return the column value as INTEGER.
     */
    std::int64_t getInt64() const noexcept;

    /**
     * Return the column value as FLOAT.
     */
    double getDouble() const noexcept;

    /**
     * Return true if the column datatype is BLOB.
     */
    bool isBlob() const noexcept;

    /**
     * Return true if the column datatype is TEXT.
     */
    bool isText() const noexcept;

    /**
     * Return true if the column datatype is INTEGER.
     */
    bool isInteger() const noexcept;

    /**
     * Return true if the column datatype is FLOAT.
     */
    bool isFloat() const noexcept;

    /**
     * Return true if the column datatype is NULL.
     */
    bool isNull() const noexcept;

private:
    sqlite3_stmt *Stmt;
    const int Index;
};

} // namespace sqldb