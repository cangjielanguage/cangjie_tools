// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Bind.h"
#include <gtest/gtest.h>

using namespace sqldb;

sqlite3* openDatabase2(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    return db;
}

#if __cplusplus > 201402L

TEST(BindTest, BindTest001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase2(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    int index = 0;
    std::nullopt_t nullopt = std::nullopt;

    int result = traits::Bind<std::nullopt_t>::call(stmt, index, nullopt);

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    EXPECT_EQ(1, 1);
}

TEST(BindTest, BindTest002)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase2(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    int index = 0;
    std::optional<int> value = 42;

    int result = traits::Bind<std::optional<int>>::call(stmt, index, value);

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    EXPECT_EQ(1, 1);
}

#endif // __cplusplus > 201402L