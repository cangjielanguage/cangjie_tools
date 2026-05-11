#ifndef CJPROF_SQLITE_H
#define CJPROF_SQLITE_H

#include <cstdint>
#include <string>
#include <sqlite3.h>

namespace cjprof {

class SQLite {
public:
    SQLite();
    ~SQLite();

    bool open(const std::string& dbPath);
    void close();

    bool execute(const std::string& sql);
    bool prepare(const std::string& sql);
    bool step();
    void finalize();

    // Bind parameters
    void bindInt(int idx, int value);
    void bindInt64(int idx, int64_t value);
    void bindText(int idx, const std::string& value);
    void bindDouble(int idx, double value);

    // Get column values
    int getColumnInt(int col);
    int64_t getColumnInt64(int col);
    std::string getColumnText(int col);
    double getColumnDouble(int col);

    // Reset prepared statement
    void reset();

private:
    sqlite3* db_ = nullptr;
    sqlite3_stmt* stmt_ = nullptr;
};

} // namespace cjprof

#endif // CJPROF_SQLITE_H
