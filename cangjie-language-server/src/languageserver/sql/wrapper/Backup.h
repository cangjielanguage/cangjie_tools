// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "Connection.h"
#include "SQLiteAPI.h"

#include <string>

namespace sqldb {
/**
 * Backup operation between source and destination SQLite databases.
 */
class Backup {
public:
    /**
     * Create backup operation object from SQLite native handle. The backup
     * object will manage the lifetime of the passed native handle.
     */
    explicit Backup(sqlite3_backup *P) noexcept;

    /**
     * Release all resources associated with managed SQLite native handle.
     */
    ~Backup() noexcept;

    /**
     * Backup objects are move-constructible.
     */
    Backup(Backup &&Other) noexcept;

    /**
     * Backup objects are move-assignable.
     */
    Backup &operator=(Backup &&Other) noexcept;

    /**
     * Backup objects are not copy-constructible.
     */
    Backup(const Connection &) = delete;

    /**
     * Backup objects are not copy-assignable.
     */
    Backup &operator=(const Backup &) = delete;

    /**
     * Exchange SQLite native handle of the backup with those of Other
     * backup object.
     */
    void swap(Backup &Other) noexcept;

    /**
     * Return the total number of pages in the source database.
     */
    int getTotalPageCount() const noexcept;

    /**
     * Return the number of pages still to be backed up.
     */
    int getRemainingPages() const noexcept;

    /**
     * Copy up to Pages between the source and the destination databases.
     * If Pages is negative, all remaining source pages are copied.
     */
    void step(int Pages);

    /**
     * Copy all pages between the source and the destination databases.
     */
    void execute();

private:
    sqlite3_backup *P = nullptr;
};

/**
 * Creates SQLite database backup operation object.
 */
Backup backup(Connection &Dst, Connection &Src, const std::string &Name = "main");

} // namespace sqldb