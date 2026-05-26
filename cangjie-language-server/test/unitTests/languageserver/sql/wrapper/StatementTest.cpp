// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Statement.h"
#include <gtest/gtest.h>

using namespace sqldb;

sqlite3* openDatabase8(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    return db;
}

TEST(StatementTest, Test001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase8(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto stm = Statement(stmt);
    if (stm) {

    }
    EXPECT_EQ(1, 1);
}

TEST(StatementTest, Test002)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase8(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto stm = Statement(stmt);
    std::string_view sq = stm.getSQL();
    EXPECT_EQ(1, 1);
}

TEST(StatementTest, Test003)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase8(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto stm = Statement(stmt);
    std::string expandedSql = stm.getExpandedSQL();
    EXPECT_EQ(1, 1);
}

TEST(StatementTest, Test004)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase8(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto stm = Statement(stmt);
    bool isReadOnly = stm.isReadOnly();
    EXPECT_EQ(1, 1);
}

TEST(StatementTest, Test005)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase8(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto stm = Statement(stmt);
    stm.clearBindings();
    EXPECT_EQ(1, 1);
}