// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "Connection.h"

#ifndef SQLITE_EXTENSION_API
#error "This file is intended for use by loadable extensions only"
#endif

/**
 * Register an initialization function for SQLite loadable extension.
 */
#define SQLDB_REGISTER_EXTENSION(Name, InitializeFunction)                                                             \
    sqldb::RegisterExtension Register##Name##Extension(InitializeFunction)

namespace sqldb {
struct RegisterExtension {
    RegisterExtension(void (*InitializeFunction)(Connection &));
};

} // namespace sqldb