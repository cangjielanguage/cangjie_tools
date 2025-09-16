// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Configure.h"
#include "Exception.h"

#include "sqlite3.h"

namespace sqldb {

static void logCallback(void *P, int ErrCode, const char *Msg) { reinterpret_cast<LogCallback>(P)(ErrCode, Msg); }

void setLogCallback(LogCallback Callback)
{
    int RC = sqlite3_config(SQLITE_CONFIG_LOG, logCallback, reinterpret_cast<void *>(Callback));
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to set log callback");
    }
}

void setSingleThreadMode()
{
    int RC = sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to set single thread mode");
    }
}

void setMultiThreadMode()
{
    int RC = sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to set multi thread mode");
    }
}

void setSerializedMode()
{
    int RC = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to set serialized mode");
    }
}

} // namespace sqldb