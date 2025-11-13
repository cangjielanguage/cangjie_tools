// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Result.h"
#include <gtest/gtest.h>

using namespace sqldb;

sqlite3* openDatabase6(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    return db;
}

TEST(ResultTest, ResultTestTest001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    auto result = Result(stmt);
    auto ans = result.getColumn(0);
    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest002)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    auto result = Result(stmt);
    auto ans = result.operator[](0);
    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest003)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    auto result = Result(stmt);
    auto ans = result.begin();
    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest004)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    auto result = Result(stmt);
    auto ans = result.end();
    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest005)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    int count = 1;
    int index = 0;
    Result::Iterator it(stmt, count, index);

    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest006)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    int count = 1;
    int index = 0;
    Result::Iterator it(stmt, count, index);
    auto row = *it;

    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest007)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    int count = 1;
    int index = 0;
    Result::Iterator it(stmt, count, index);
    ++it;

    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest008)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    int count = 1;
    int index = 0;
    Result::Iterator it(stmt, count, index);
    it++;

    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest009)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    int count = 1;
    int index = 0;
    Result::Iterator it1(stmt, count, index);
    Result::Iterator it2(stmt, count, index);
    if (it1 == it2) { }

    EXPECT_EQ(1, 1);
}

TEST(ResultTest, ResultTestTest010)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase6(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    int count = 1;
    int index = 0;
    Result::Iterator it1(stmt, count, index);
    Result::Iterator it2(stmt, count, index);
    if (it1 != it2) { }

    EXPECT_EQ(1, 1);
}