// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Transaction.h"
#include <gtest/gtest.h>

using namespace sqldb;

sqlite3* openDatabase10(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    return db;
}

TEST(TransactionTest, Test001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* data = openDatabase10(dbPath);
    Connection db = Connection(data);
    Transaction txn = Transaction(db);
    EXPECT_EQ(1, 1);
}

TEST(TransactionTest, Test002)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* data = openDatabase10(dbPath);
    Connection db = Connection(data);
    Transaction txn1(db);
    Transaction txn2((std::move(txn1)));
    EXPECT_EQ(1, 1);
}

TEST(TransactionTest, Test003)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* data1 = openDatabase10(dbPath1);
    sqlite3* data2 = openDatabase10(dbPath2);
    Connection db1 = Connection(data1);
    Connection db2 = Connection(data2);
    Transaction txn1 = Transaction(db1);
    Transaction txn2 = Transaction(db2);
    txn2 = std::move(txn1);
    EXPECT_EQ(1, 1);
}

TEST(TransactionTest, Test004)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* data1 = openDatabase10(dbPath1);
    sqlite3* data2 = openDatabase10(dbPath2);
    Connection db1 = Connection(data1);
    Connection db2 = Connection(data2);
    Transaction txn1 = Transaction(db1);
    Transaction txn2 = Transaction(db2);
    txn2.swap(txn1);
    EXPECT_EQ(1, 1);
}

TEST(TransactionTest, Test005)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* data = openDatabase10(dbPath);
    Connection db = Connection(data);
    Transaction txn1 = Transaction(db);
    txn1.commit();
    EXPECT_EQ(1, 1);
}

TEST(TransactionTest, Test006)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* data = openDatabase10(dbPath);
    Connection db = Connection(data);
    Transaction txn1 = Transaction(db);
    txn1.rollback();
    EXPECT_EQ(1, 1);
}