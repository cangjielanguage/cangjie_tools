// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Result.h"
#include "Column.h"

#include "sqlite3.h"

namespace sqldb {

Result::Result(sqlite3_stmt *Stmt) noexcept : Stmt(Stmt) {}

Result::operator bool() const noexcept { return Stmt != nullptr; }

int Result::getColumnCount() const noexcept { return sqlite3_data_count(Stmt); }

Column Result::getColumn(int Index) const noexcept { return {Stmt, Index}; }

Column Result::operator[](int Index) const noexcept { return {Stmt, Index}; }

Result::Iterator Result::begin() const noexcept { return {Stmt, getColumnCount(), 0}; }

Result::Iterator Result::end() const noexcept
{
    int Count = getColumnCount();
    return {Stmt, Count, Count};
}

Result::Iterator::Iterator(sqlite3_stmt *Stmt, int Count, int Index) noexcept : Stmt(Stmt), Count(Count), Index(Index)
{
}

Result::Iterator::reference Result::Iterator::operator*() const noexcept { return {Stmt, Index}; }

Result::Iterator &Result::Iterator::operator++() noexcept
{
    ++Index;
    return *this;
}

Result::Iterator Result::Iterator::operator++(int) noexcept { return {Stmt, Count, Index++}; }

bool operator==(const Result::Iterator &LHS, const Result::Iterator &RHS) noexcept
{
    return LHS.Stmt == RHS.Stmt && LHS.Index == RHS.Index;
}

bool operator!=(const Result::Iterator &LHS, const Result::Iterator &RHS) noexcept
{
    return LHS.Stmt != RHS.Stmt || LHS.Index != RHS.Index;
}

} // namespace sqldb