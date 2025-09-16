// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include <cstddef>

namespace sqldb {

/**
 * Return the number of bytes of heap memory currently allocated by SQLite.
 */
std::size_t getMemoryUsed() noexcept;

/**
 * Return the maximum value of getMemoryUsed() since the high-water mark
 * was last reset.
 * The memory high-water mark is reset to the current value of
 * getMemoryUsed() if and only if the Reset parameter is set to
 * true value. The value returned after the call is the high-water mark
 * prior to the reset.
 */
std::size_t getMemoryHighWater(bool Reset = false) noexcept;

/**
 * Attempt to free as much memory used by database library as possible. Returns
 * the number of bytes of heap memory actually freed.
 */
std::size_t releaseMemory() noexcept;

/**
 * Query the soft limit on the amount of heap memory that may be allocated.
 * Memory allocations will fail when the hard heap limit is reached.
 */
std::size_t getSoftHeapLimit() noexcept;

/**
 * Query the hard limit on the amount of heap memory that may be allocated.
 */
std::size_t getHardHeapLimit() noexcept;

/**
 * Set the soft limit on the amount of heap memory that may be allocated by
 * SQLite. SQLite strives to keep heap memory utilization below the soft heap
 * limit by reducing the number of pages held in the page cache as heap memory
 * usages approaches the limit.
 * The soft heap limit may not be greater than the hard heap limit. If the hard
 * heap limit is enabled and if the soft heap limit is set to a value that is
 * greater than the hard heap limit, the the soft heap limit is set to the
 * value of the hard heap limit.
 */
void setSoftHeapLimit(std::size_t N); // In bytes.

/**
 * Set a hard upper bound of N bytes on the amount of memory that will be
 * allocated. Memory allocations will fail when the hard heap limit is reached.
 */
void setHardHeapLimit(std::size_t N); // In bytes.

} // namespace sqldb