// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_ARKSCHEDULER_H
#define LSPSERVER_ARKSCHEDULER_H

#include <map>
#include "ArkThreading.h"
#include "ArkASTWorker.h"

namespace ark {
class ArkScheduler {
public:
    explicit ArkScheduler(Callbacks *c);
    ~ArkScheduler() noexcept;

    void Update(const ParseInputs &inputs, NeedDiagnostics needDiag) const;

    void RunWithAST(const std::string &name, const std::string &file, std::function<void(InputsAndAST)> action) const;

    void RunWithASTCache(
        const std::string &name, const std::string &file, Position pos, std::function<void(InputsAndAST)> action) const;

private:
    Semaphore barrier;
    AsyncTaskRunner *workerThreads = nullptr;
    Callbacks *callback = nullptr;

    // worker will be freed after polling thread exit
    // we call worker->stop to free worker
    ArkASTWorker *worker = nullptr;
};
} // namespace ark

#endif // LSPSERVER_ARKSCHEDULER_H
