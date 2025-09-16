// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_INDEX_THREAD_MEMOIZE_H
#define LSPSERVER_INDEX_THREAD_MEMOIZE_H

#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include "../logger/Logger.h"

namespace ark {
namespace lsp {
const unsigned int EXTRA_THREAD_COUNT = 3; // main thread and message thread
const unsigned int HARDWARE_CONCURRENCY_COUNT = std::thread::hardware_concurrency();
const unsigned int MAX_THREAD_COUNT = HARDWARE_CONCURRENCY_COUNT > EXTRA_THREAD_COUNT ?
                                      HARDWARE_CONCURRENCY_COUNT - EXTRA_THREAD_COUNT : 1;

bool ShutdownRequested();

/**
 * Checks whether notifyShutdown() or requestShutdown() was called.
 * This function is threadsafe and signal-safe.
 */
bool ShutdownPending();

/**
 * Variation of Memoize which maintains a set of thread local caches.
 */
template <typename container> class ThreadMemoize {
public:
    ThreadMemoize() : capacity(MAX_THREAD_COUNT) {}

    template <typename F> auto &Get(F &&compute)
    {
        auto tid = GetThreadID();
        std::lock_guard<std::mutex> lock(threadsMu);
        return Put(tid, std::forward<F>(compute));
    }

    void EraseThreadCache()
    {
        auto tid = GetThreadID();
        std::lock_guard<std::mutex> lock(threadsMu);
        auto it = threadsCache.find(tid);
        if (it == threadsCache.end()) {
            return;
        }
        threadList.erase(it->second);
        threadsCache.erase(it);
    }

private:
    static std::thread::id GetThreadID()
    {
        static thread_local auto threadID = std::this_thread::get_id();
        return threadID;
    }

    template <typename F>
    container& Put(const std::thread::id& tid, F&& compute)
    {
        auto it = threadsCache.find(tid);
        if (it != threadsCache.end()) {
            threadList.splice(threadList.begin(), threadList, it->second);
            return it->second->second;
        }

        if (threadList.size() >= capacity) {
            auto last = threadList.end();
            --last;
            threadsCache.erase(last->first);
            threadList.pop_back();
        }

        threadList.emplace_front(tid, compute());
        threadsCache[tid] = threadList.begin();
        return threadList.begin()->second;
    }

    size_t capacity;

    std::mutex threadsMu;
    std::list<std::pair<std::thread::id, container>> threadList;
    std::unordered_map<std::thread::id,
        typename std::list<std::pair<std::thread::id, container>>::iterator> threadsCache;
};

} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_THREAD_MEMOIZE_H
