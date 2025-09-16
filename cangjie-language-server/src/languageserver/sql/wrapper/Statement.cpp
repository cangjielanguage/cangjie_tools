// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Statement.h"
#include "Exception.h"

#include "ScopeExit.h"

#include "sqlite3.h"

namespace sqldb {

Statement::Statement(sqlite3_stmt *Stmt) noexcept : Stmt(Stmt) {}

Statement::~Statement() noexcept { sqlite3_finalize(Stmt); }

Statement::Statement(Statement &&Other) noexcept { swap(Other); }

Statement &Statement::operator=(Statement &&Other) noexcept
{
    swap(Other);
    if (this == &Other) {
        return *this;
    }
    return *this;
}

void Statement::swap(Statement &Other) noexcept { std::swap(Stmt, Other.Stmt); }

Statement::operator bool() const noexcept { return Stmt != nullptr; }

std::string_view Statement::getSQL() const noexcept { return sqlite3_sql(Stmt); }

std::string Statement::getExpandedSQL() const
{
    char *SQL = nullptr;
    ScopeExit FreeSQL = [&]() noexcept { sqlite3_free(SQL); };
    if ((SQL = sqlite3_expanded_sql(Stmt)) != nullptr) {
        return std::string(SQL);
    }
    return {};
}

bool Statement::isReadOnly() const noexcept { return sqlite3_stmt_readonly(Stmt); }

bool Statement::isBusy() const noexcept { return sqlite3_stmt_busy(Stmt); }

void Statement::clearBindings() noexcept { sqlite3_clear_bindings(Stmt); }

void Statement::reset()
{
    if (sqlite3_reset(Stmt) != SQLITE_OK) {
        throw Exception(sqlite3_db_handle(Stmt), "Failed to reset statement");
    }
}

Result Statement::step()
{
    switch (sqlite3_step(Stmt)) {
        case SQLITE_ROW:
            return Result(Stmt);
        case SQLITE_DONE:
            sqlite3_reset(Stmt);
            sqlite3_clear_bindings(Stmt);
            return Result();
        default:
            throw Exception(sqlite3_db_handle(Stmt), "Failed to execute statement \"" + getExpandedSQL() + "\"");
    }
}

void Statement::execute(std::function<bool(Result)> Callback)
{
    ScopeExit Cleanup = [this]() noexcept {
        sqlite3_clear_bindings(Stmt);
        sqlite3_reset(Stmt);
    };
    while (Result Row = step()) {
        if (!Callback(Row)) {
            break;
        }
    }
}

void Statement::execute()
{
    ScopeExit Cleanup = [this]() noexcept {
        sqlite3_clear_bindings(Stmt);
        sqlite3_reset(Stmt);
    };
    while (step()) {}
}

} // namespace sqldb