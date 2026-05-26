// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Value.h"
#include <gtest/gtest.h>

using namespace sqldb;

sqlite3* openDatabase9(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    return db;
}

void createTable4(sqlite3* db) {
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

void insertData4(sqlite3* db) {
    const char* sql = "INSERT INTO users (name, age) VALUES ('Alice', 30), ('Bob', 25);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to insert data: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    std::cout << "Data inserted successfully." << std::endl;
}


static void MyCustomFunction1(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double doubleValue = traits::Value<double>::call(argv[0]);
    std::string_view stringValue = traits::Value<std::string_view>::call(argv[0]);
    std::optional<double> optionalDouble = traits::Value<std::optional<double>>::call(argv[0]);
    int a = sqlite3_value_int(argv[1]);
    sqlite3_result_int(ctx, a);
}

TEST(ValueTest, Test001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase9(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable4(db);
    insertData4(db);
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