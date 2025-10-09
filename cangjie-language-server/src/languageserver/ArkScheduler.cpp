// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ArkScheduler.h"
#include "common/BasicHelper.h"

namespace ark {
    ArkScheduler::ArkScheduler(Callbacks *c) : barrier(1), workerThreads(new AsyncTaskRunner()), callback(c),
                                               worker(ArkASTWorker::Create(*workerThreads, barrier, callback)) {}

ArkScheduler::~ArkScheduler() noexcept
{
    if (worker) {
        worker->Stop();
    }
    if (workerThreads) {
        delete workerThreads;
        workerThreads = nullptr;
    }
}

void ArkScheduler::Update(const ParseInputs &inputs, NeedDiagnostics needDiag) const
{
    worker->Update(inputs, needDiag);
}

void ArkScheduler::RunWithAST(const std::string &name, const std::string &file,
                              std::function<void(InputsAndAST)> action) const
{
    worker->RunWithAST(name, file, std::move(action), NeedDiagnostics::YES);
}

void ArkScheduler::RunWithASTCache(
    const std::string &name, const std::string &file, Position pos, std::function<void(InputsAndAST)> action) const
{
    worker->RunWithASTCache(name, file, pos, std::move(action));
}
} // namespace ark
