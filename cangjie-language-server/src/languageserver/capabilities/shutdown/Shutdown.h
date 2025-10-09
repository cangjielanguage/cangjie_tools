// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_SHUTDOWN_H
#define LSPSERVER_SHUTDOWN_H

#include <iostream>

namespace ark {
bool ShutdownRequested();

void RequestShutdown();

template<typename Fun, typename Ret = decltype(std::declval<Fun>()())>
Ret RetryAfterSignalUnlessShutdown(const std::enable_if_t<true, Ret> &fail, const Fun &func)
{
    Ret res;
    do {
        errno = 0;
        res = func();
    } while (res == fail && errno == EINTR);
    return res;
}
} // namespace ark

#endif // LSPSERVER_SHUTDOWN_H
