// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_CRASHREPORTER_H
#define LSPSERVER_CRASHREPORTER_H

#ifdef __linux__
#include <execinfo.h>
#endif
#include <csignal>

namespace ark {
class CrashReporter {
public:
#ifdef __linux__
    static void SignalHandler(int sig);
#endif
    static void RegisterHandlers();

private:
    bool isActive;
};
} // namespace ark

#endif // LSPSERVER_CRASHREPORTER_H