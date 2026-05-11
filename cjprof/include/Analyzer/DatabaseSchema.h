#ifndef CJPROF_SCHEMA_H
#define CJPROF_SCHEMA_H

#include <string>

namespace cjprof {

class DatabaseSchema {
public:
    static const char* getCreateTablesSQL();
    static const char* getCreateIndexesSQL();
};

} // namespace cjprof

#endif // CJPROF_SCHEMA_H
