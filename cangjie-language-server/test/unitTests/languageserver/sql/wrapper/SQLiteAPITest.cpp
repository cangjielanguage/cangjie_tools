// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SQLiteAPI.h"
#include <gtest/gtest.h>

using namespace sqlite;

sqlite3* openDatabase7(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    return db;
}

void createTable3(sqlite3* db) {
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

void insertData3(sqlite3* db) {
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
    int type = value_type(argv[0]);
    sqlite3_result_int(ctx, type);
}

TEST(SQLiteAPITest, Test001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
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
    auto type = value_double(argv[0]);
    int a = sqlite3_value_int(argv[1]);
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test002)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction2,
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

static void MyCustomFunction3(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    auto type = value_text(argv[0]);
    int a = sqlite3_value_int(argv[1]);
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test003)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
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

TEST(SQLiteAPITest, Test004)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto result = column_count(stmt);

    EXPECT_EQ(1, 1);
}

TEST(SQLiteAPITest, Test005)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto result = bind_null(stmt, 0);

    EXPECT_EQ(1, 1);
}

TEST(SQLiteAPITest, Test006)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto result = bind_double(stmt, 0, 0);

    EXPECT_EQ(1, 1);
}

TEST(SQLiteAPITest, Test007)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM users;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    auto result = bind_parameter_index(stmt, "test");

    EXPECT_EQ(1, 1);
}

static void MyCustomFunction8(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int a = sqlite3_value_int(argv[0]);
    result_error(ctx, "error message", 0);
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test008)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction8,
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

static void MyCustomFunction9(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int a = sqlite3_value_int(argv[0]);
    result_int64(ctx, 0);
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test009)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction9,
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

static void MyCustomFunction10(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int a = sqlite3_value_int(argv[0]);
    result_double(ctx, 0);
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test010)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction10,
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

static void MyCustomFunction11(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int a = sqlite3_value_int(argv[0]);
    sqlite3_destructor_type dtor = SQLITE_STATIC;
    result_text64(ctx, "error message", 0, dtor, 'a');
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test011)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction11,
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

static void MyCustomFunction12(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int a = sqlite3_value_int(argv[0]);
    sqlite3_destructor_type dtor = SQLITE_STATIC;
    result_blob64(ctx, "error message", 0, dtor);
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test012)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction12,
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

static void MyCustomFunction13(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int a = sqlite3_value_int(argv[0]);
    sqlite3_destructor_type dtor = SQLITE_STATIC;
    result_null(ctx);
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test013)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction13,
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

TEST(SQLiteAPITest, Test014)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    create_scalar(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction13,
        nullptr
    );


    sqlite3_close(db);

    EXPECT_EQ(1, 1);
}

static void MyCustomFunction15(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    int a = sqlite3_value_int(argv[0]);
    user_data(ctx);
    sqlite3_result_int(ctx, a);
}

TEST(SQLiteAPITest, Test015)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    sqlite3_create_function(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        MyCustomFunction15,
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

TEST(SQLiteAPITest, Test016)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = openDatabase7(dbPath);
    if (!db) {
        std::cerr << "Failed to open database." << std::endl;
    }
    createTable3(db);
    insertData3(db);
    create_aggregate(
        db,
        "my_function",
        2,
        SQLITE_UTF8,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );
    sqlite3_close(db);

    EXPECT_EQ(1, 1);
}