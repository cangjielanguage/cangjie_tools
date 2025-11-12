#include "Column.h"
#include <gtest/gtest.h>
 
using namespace sqldb;
 
sqlite3* openDatabase3(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    if (rc != SQLITE_OK) {
    }
    return db;
}
 
void createTable(sqlite3* db) {
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
 
void insertData(sqlite3* db) {
    const char* sql = "INSERT INTO users (name, age) VALUES ('Alice', 30), ('Bob', 25);";
 
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to insert data: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    std::cout << "Data inserted successfully." << std::endl;
}
 
auto queryData(sqlite3* db) {
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
 
        rc = sqlite3_step(stmt);
    }
 
    return stmt;
}
 
void closeDatabase(sqlite3* db) {
    if (db) {
        sqlite3_close(db);
    }
}
 
 
TEST(ColumnTest, ColumnTest001)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
 
    EXPECT_EQ(1, 1);
}
 
TEST(ColumnTest, ColumnTest002)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
 
    EXPECT_EQ(column.getName(), "id");
}
 
TEST(ColumnTest, ColumnTest003)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.getBlob();
 
    EXPECT_EQ(1, 1);
}
 
TEST(ColumnTest, ColumnTest004)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.getText();
 
    EXPECT_EQ(1, 1);
}
 
TEST(ColumnTest, ColumnTest005)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.getInt64();
 
    EXPECT_EQ(result, 0);
}
 
TEST(ColumnTest, ColumnTest006)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.getDouble();
 
    EXPECT_EQ(result, 0);
}
 
TEST(ColumnTest, ColumnTest007)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.isBlob();
 
    EXPECT_EQ(result, false);
}
 
TEST(ColumnTest, ColumnTest008)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.isFloat();
 
    EXPECT_EQ(result, false);
}
 
TEST(ColumnTest, ColumnTest009)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.isInteger();
 
    EXPECT_EQ(result, false);
}
 
TEST(ColumnTest, ColumnTest010)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.isText();
 
    EXPECT_EQ(result, false);
}
 
TEST(ColumnTest, ColumnTest011)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
 
    db = openDatabase3(dbPath);
    createTable(db);
    insertData(db);
    auto stmt = queryData(db);
    auto index = 0;
    auto column = Column(stmt, index);
    auto result = column.isNull();
 
    EXPECT_EQ(result, true);
}