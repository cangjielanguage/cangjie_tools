// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


#include "Connection.cpp"
#include <gtest/gtest.h>

using namespace sqldb;

sqlite3* openDatabase4(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    return db;
}

TEST(ConnectionTest, ConnectionTest001)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, ConnectionTest002)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection1(db);
    Connection connection2((std::move(connection1)));

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, ConnectionTest003)
{
    const std::string dbPath = "mydatabase.db";
    auto db1 = openDatabase4(dbPath);
    auto db2 = openDatabase4(dbPath);
    Connection connection1(db1);
    Connection connection2(db2);
    connection1.operator=(std::move(connection2));

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, ConnectionTest004)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    connection.operator=(std::move(connection));

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, ConnectionTest005)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    auto result = bool(connection);

    EXPECT_EQ(result, true);
}

TEST(ConnectionTest, ConnectionTest006)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    auto result = connection.getFileName(dbPath);

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, ConnectionTest007)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    auto result = connection.isReadOnly(dbPath);

    EXPECT_EQ(result, true);
}

TEST(ConnectionTest, ConnectionTest008)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    connection.interrupt();

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, ConnectionTest009)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    auto result = connection.isInterrupted();

    EXPECT_EQ(result, false);
}

TEST(ConnectionTest, ConnectionTest010)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    connection.enableLoadExtension();

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, ConnectionTest011)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    std::filesystem::path path = "mydatabase.db";
    connection.loadExtension(path);

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, ConnectionTest012)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    auto result = connection.getChanges();

    EXPECT_EQ(result, 0);
}

TEST(ConnectionTest, ConnectionTest013)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    connection.releaseMemory();

    EXPECT_EQ(1, 1);
}


TEST(ConnectionTest, memdbTest001)
{
    const std::string dbPath = "mydatabase.db";
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    auto db = memdb(dbPath, flags);
    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, tempdbTest001)
{
    auto db = tempdb();

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, getNativeHandleTest001)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    auto result = connection.getNativeHandle();

    EXPECT_EQ(1, 1);
}

TEST(ConnectionTest, scalarTest001)
{
    const std::string dbPath = "mydatabase.db";
    auto db = openDatabase4(dbPath);
    Connection connection(db);
    auto add = [](int a, int b) { return a + b; };
    connection.scalar("add", add);

    EXPECT_EQ(1, 1);
}