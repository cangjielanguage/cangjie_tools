// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <atomic>
#include "../../logger/Logger.h"
#include "Shutdown.h"

namespace ark {
using namespace Cangjie;
static std::atomic<bool> g_shutdownRequested = {false};

void RequestShutdown()
{
    if (g_shutdownRequested.exchange(true)) {
        // This is the second shutdown request. Exit hard.
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, "receive the second shutdown request.");
    }
}

bool ShutdownRequested()
{
    return g_shutdownRequested;
}
} // namespace ark
