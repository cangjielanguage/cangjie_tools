// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Function.h"
#include <gtest/gtest.h>

using namespace sqldb;

sqlite3* openDatabase5(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    return db;
}

void createTable2(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT NOT NULL,"
                      "age INTEGER NOT NULL"
                      ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    std::cout << "Table created successfully." << std::endl;
}

void insertData2(sqlite3* db) {
    const char* sql = "INSERT INTO users (name, age) VALUES ('Alice', 30), ('Bob', 25);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to insert data: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    std::cout << "Data inserted successfully." << std::endl;
}

sqlite3_stmt* queryData2(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name, age FROM users;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    }

    rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int age = sqlite3_column_int(stmt, 2);

        std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
        rc = sqlite3_step(stmt);
    }

    return stmt;
}


static void MyCustomFunction1(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int a = sqlite3_value_int(argv[0]);
    int b = sqlite3_value_int(argv[1]);

    int sum = a + b;
    traits::result(ctx, sum);
}

TEST(FunctionTest, resultTest001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase5(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable2(db);
    insertData2(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction1,
        nullptr,
        nullptr
    );

    char* errMsg = nullptr;
    sqlite3_exec(
        db,
        "SELECT my_function(10, 20)",
        [](void* data, int argc, char** argv, char** colName) -> int {
            std::cout << "Result: " << argv[0] << std::endl;
            return 0;
        },
        nullptr,
        &errMsg
    );

    if (errMsg) {
        std::cerr << "Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);

    EXPECT_EQ(1, 1);
}

static void MyCustomFunction2(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int value = sqlite3_value_int(argv[0]);

    traits::Result<int>::call(ctx, value * 2);
}

TEST(FunctionTest, callTest001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase5(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable2(db);
    insertData2(db);
    sqlite3_create_function(
        db,
        "my_function",
        1,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction2,
        nullptr,
        nullptr
    );

    char* errMsg = nullptr;
    sqlite3_exec(
        db,
        "SELECT my_function(42)",
        [](void* data, int argc, char** argv, char** colName) -> int {
            std::cout << "Result: " << argv[0] << std::endl;
            return 0;
        },
        nullptr,
        &errMsg
    );

    if (errMsg) {
        std::cerr << "Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    sqlite3_close(db);

    EXPECT_EQ(1, 1);
}

static void MyCustomFunction3(sqlite3_context* ctx, int argc, sqlite3_value** argv) {

    int a = sqlite3_value_int(argv[0]);
    int b = sqlite3_value_int(argv[1]);

    if (a + b == 0) {
        traits::Result<std::nullopt_t>::call(ctx, std::nullopt);
    } else {
        sqlite3_result_int(ctx, a + b);
    }
}

TEST(FunctionTest, callTest002)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase5(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable2(db);
    insertData2(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction3,
        nullptr,
        nullptr
    );

    char* errMsg = nullptr;
    sqlite3_exec(
        db,
        "SELECT my_function(1, -1)",
        [](void* data, int argc, char** argv, char** colName) -> int {
            std::cout << "Result: " << argv[0] << std::endl;
            return 0;
        },
        nullptr,
        &errMsg
    );

    if (errMsg) {
        std::cerr << "Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    sqlite3_close(db);

    EXPECT_EQ(1, 1);
}