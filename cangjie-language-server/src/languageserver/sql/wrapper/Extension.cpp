// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifdef SQLITE_EXTENSION_API

#include "Extension.h"
#include "Connection.h"

#include "ScopeExit.h"

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

#include <exception>
#include <vector>

namespace sqldb {

static std::vector<void (*)(Connection &)> &getInitializeFunctions()
{
    static std::vector<void (*)(Connection &)> InitializeFunctions;
    return InitializeFunctions;
}

RegisterExtension::RegisterExtension(void (*InitializeFunction)(class Connection &))
{
    getInitializeFunctions().push_back(InitializeFunction);
}

} // namespace sqldb

#ifdef _WIN32
#define SQLDB_EXPORT_API __declspec(dllexport)
#else
#define SQLDB_EXPORT_API
#endif

/**
 * NOLINTNEXTLINE(readability-identifier-naming)
 */
extern "C" int SQLDB_EXPORT_API sqlite3_extension_init(sqlite3 *DB, char **ErrMsg, const sqlite3_api_routines *API)
{
    int RC = sqlite::OK;
    SQLITE_EXTENSION_INIT2(API);
    sqldb::Connection Connection(DB);
    sqldb::ScopeExit ReleaseDB = [&]() noexcept { DB = Connection.release(); };
    try {
        for (auto &InitializeExtension : sqldb::getInitializeFunctions())
            InitializeExtension(Connection);
    } catch (const std::exception &Exception) {
        if (ErrMsg != nullptr) {
            *ErrMsg = sqlite3_mprintf("%s", Exception.what());
        }
        RC = sqlite::Error;
    } catch (...) {
        if (ErrMsg != nullptr) {
            *ErrMsg = sqlite3_mprintf("Unknown error");
        }
        RC = sqlite::Error;
    }
    return RC;
}

#endif