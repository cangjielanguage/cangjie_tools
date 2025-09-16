// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Backup.h"
#include "Connection.h"
#include "Exception.h"

#include "sqlite3.h"

namespace sqldb {

Backup::Backup(sqlite3_backup *P) noexcept : P(P) {}

Backup::~Backup() noexcept { sqlite3_backup_finish(P); }

Backup::Backup(Backup &&Other) noexcept { swap(Other); }

Backup &Backup::operator=(Backup &&Other) noexcept
{
    swap(Other);
    if (this == &Other) {
        return *this;
    }
    return *this;
}

void Backup::swap(Backup &Other) noexcept { std::swap(P, Other.P); }

int Backup::getTotalPageCount() const noexcept { return sqlite3_backup_pagecount(P); }

int Backup::getRemainingPages() const noexcept { return sqlite3_backup_remaining(P); }

void Backup::step(int Pages)
{
    int RC = sqlite3_backup_step(P, Pages);
    if (RC != SQLITE_OK && RC != SQLITE_DONE) {
        throw Exception(RC, "Failed to perform database backup");
    }
}

void Backup::execute() { step(-1); }

Backup backup(Connection &Dst, Connection &Src, const std::string &Name)
{
    sqlite3_backup *P = sqlite3_backup_init(Dst.getNativeHandle(), Name.c_str(), Src.getNativeHandle(), Name.c_str());
    if (P == nullptr) {
        throw Exception(Dst.getNativeHandle(), "Failed to initialize database backup");
    }
    return Backup(P);
}

} // namespace sqldb