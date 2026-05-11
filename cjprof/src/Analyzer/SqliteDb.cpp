#include "Analyzer/SqliteDb.h"
#include "Analyzer/Logger.h"
#include <cstring>

namespace cjprof {

SQLite::SQLite() = default;

SQLite::~SQLite() {
    close();
}

bool SQLite::open(const std::string& dbPath) {
    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc) {
        LOG_ERROR("Cannot open database: {}", sqlite3_errmsg(db_));
        return false;
    }
    // Enable WAL mode for better performance
    execute("PRAGMA journal_mode=WAL;");
    return true;
}

void SQLite::close() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool SQLite::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("SQL error: {}", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool SQLite::prepare(const std::string& sql) {
    if (stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: {}", sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

bool SQLite::step() {
    if (!stmt_) return false;
    int rc = sqlite3_step(stmt_);
    return rc == SQLITE_ROW;
}

bool SQLite::stepDone() {
    if (!stmt_) return false;
    int rc = sqlite3_step(stmt_);
    return rc == SQLITE_DONE;
}

void SQLite::finalize() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
}

void SQLite::bindInt(int idx, int value) {
    sqlite3_bind_int(stmt_, idx, value);
}

void SQLite::bindInt64(int idx, int64_t value) {
    sqlite3_bind_int64(stmt_, idx, value);
}

void SQLite::bindText(int idx, const std::string& value) {
    sqlite3_bind_text(stmt_, idx, value.c_str(), -1, SQLITE_TRANSIENT);
}

void SQLite::bindDouble(int idx, double value) {
    sqlite3_bind_double(stmt_, idx, value);
}

int SQLite::getColumnInt(int col) {
    return sqlite3_column_int(stmt_, col);
}

int64_t SQLite::getColumnInt64(int col) {
    return sqlite3_column_int64(stmt_, col);
}

std::string SQLite::getColumnText(int col) {
    const unsigned char* text = sqlite3_column_text(stmt_, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}

double SQLite::getColumnDouble(int col) {
    return sqlite3_column_double(stmt_, col);
}

void SQLite::reset() {
    if (stmt_) {
        sqlite3_reset(stmt_);
        sqlite3_clear_bindings(stmt_);
    }
}

} // namespace cjprof
