// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ArkThreading.h"
#include <atomic>

namespace ark {

Semaphore::Semaphore(std::size_t maxLocks) : freeSlots(maxLocks) {}

bool Semaphore::try_lock()
{
    std::unique_lock<std::mutex> lock(mutexSemaphore);
    if (freeSlots > 0) {
        --freeSlots;
        return true;
    }
    return false;
}

void Semaphore::lock()
{
    std::unique_lock<std::mutex> lock(mutexSemaphore);
    slotsChanged.wait(lock, [this]() { return freeSlots > 0; });
    --freeSlots;
}

void Semaphore::unlock()
{
    std::unique_lock<std::mutex> lock(mutexSemaphore);
    ++freeSlots;
    lock.unlock();
    slotsChanged.notify_one();
}

void Wait(std::unique_lock<std::mutex> &lock, std::condition_variable &cv, const Deadline deadline)
{
    if (deadline == Deadline::Zero()) {
        return;
    }
    if (deadline == Deadline::Infinity()) {
        return cv.wait(lock);
    }
    (void)cv.wait_until(lock, deadline.Time());
}
} // namespace ark

