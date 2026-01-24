// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_ARKASTWORKER_H
#define LSPSERVER_ARKASTWORKER_H

#include <queue>
#include <vector>
#include <map>
#include <sstream>
#include "ArkThreading.h"
#include "ArkAST.h"
#include "CompilerCangjieProject.h"
#include "capabilities/semanticHighlight/SemanticHighlightImpl.h"
#include "common/Callbacks.h"
#include "capabilities/diagnostic/LSPDiagObserver.h"

namespace ark {
class ArkASTWorker;
class AsyncTaskRunner;

struct InputsAndAST {
    const ParseInputs &inputs;
    ArkAST *ast;
    std::string onEditFile;
    bool useASTCache;
};

using ThreadFunc = std::function<void()>;
struct ThreadData {
    ArkASTWorker *worker;
    AsyncTaskRunner *runner;
    ThreadFunc action;
};

class ArkASTWorker {
public:
    ArkASTWorker(Semaphore &semaphore, Callbacks *c);
    static ArkASTWorker* Create(AsyncTaskRunner &asyncTaskRunner, Semaphore &semaphore, Callbacks *c);
    ~ArkASTWorker();

    void Update(const ParseInputs &inputs, NeedDiagnostics needDiag);

    bool GetStatus() const
    {
        return isRun;
    }

    void RunWithAST(const std::string &name, const std::string &file,
                    std::function<void(InputsAndAST)> action, NeedDiagnostics needDiag);

    void RunWithASTCache(
        const std::string &name, const std::string &file, Position pos, std::function<void(InputsAndAST)> action);

    void DoCompletionWithASTCache(
        const std::string &name, const std::string &file, Position pos, std::function<void(InputsAndAST)> action);

    void Stop() noexcept;

private:
    void Run();

    Deadline ScheduleLocked();

    bool ShouldSkipHeadLocked() const;

    void StartTask(std::string name, std::function<void()> task, NeedDiagnostics needDiag);

    struct Request {
        std::function<void()> action;
        std::string name;
        NeedDiagnostics updateType = NeedDiagnostics::AUTO;
    };

    Semaphore &barrier;
    // Guards members used by both TUScheduler and the worker thread.
    mutable std::mutex mutex;
    std::mutex editMutex;
    std::deque<Request> requests;           /* GUARDED_BY(Mutex) */
    Request currentRequest; /* GUARDED_BY(Mutex) */
    mutable std::condition_variable requestsCV;

    bool done = false;

    bool isRun = false;

    Callbacks *callback;

    std::string onEditName;

    bool isCompleteRunning = false;
    mutable std::mutex completionMtx;
    std::function<void()> waitingCompletionTask = nullptr;
};

class AsyncTaskRunner {
public:
    AsyncTaskRunner();
    // Destructor waits for all pending tasks to finish.
    ~AsyncTaskRunner() noexcept;

    void Wait() const noexcept
    {
        Wait(Deadline::Infinity());
    }
    void Wait(const Deadline &deadline) const;

#ifdef __APPLE__
    static void* thread_routine(void* arg)
    {
        auto* data = static_cast<ThreadData*>(arg);
        data->action();
        if (data->worker) {
            delete data->worker;
            data->worker = nullptr;
        }
        data->action = nullptr;
        {
            std::lock_guard<std::mutex> lock(data->runner->mutex);
            --data->runner->inFlightTasks;
            if (!data->runner->inFlightTasks) {
                data->runner->tasksReachedZero.notify_all();
            }
        }
        delete data;
        return nullptr;
    }
#endif
    // The name is used for tracing and debugging (e.g. to name a spawned thread).
    void RunAsync(const std::string &name, std::function<void()> action, ArkASTWorker *worker);

private:
    mutable std::mutex mutex;
    mutable std::condition_variable tasksReachedZero;
    std::size_t inFlightTasks;
};
} // namespace ark

#endif // LSPSERVER_ARKASTWORKER_H
