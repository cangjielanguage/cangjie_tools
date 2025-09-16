// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Memory.h"

#include "sqlite3.h"

#include <stdexcept>

namespace sqldb {

std::size_t getMemoryUsed() noexcept { return static_cast<std::size_t>(sqlite3_memory_used()); }

std::size_t getMemoryHighWater(bool Reset) noexcept
{
    return static_cast<std::size_t>(sqlite3_memory_highwater(Reset));
}

std::size_t releaseMemory() noexcept { return static_cast<std::size_t>(sqlite3_release_memory(-1)); }

std::size_t getSoftHeapLimit() noexcept { return static_cast<std::size_t>(sqlite3_soft_heap_limit64(-1)); }

std::size_t getHardHeapLimit() noexcept { return static_cast<std::size_t>(sqlite3_hard_heap_limit64(-1)); }

void setSoftHeapLimit(std::size_t N)
{
    if (sqlite3_soft_heap_limit64(static_cast<sqlite_int64>(N)) < 0) {
        throw std::runtime_error("Failed to set SQLite soft heap limit");
    }
}

void setHardHeapLimit(std::size_t N)
{
    if (sqlite3_hard_heap_limit64(static_cast<sqlite_int64>(N)) < 0) {
        throw std::runtime_error("Failed to set SQLite hard heap limit");
    }
}

} // namespace sqldb