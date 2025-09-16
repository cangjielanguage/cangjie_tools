// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Transaction.h"
#include "Connection.h"

#include "Invoke.h"

#include <utility>

namespace sqldb {

Transaction::Transaction(Connection &DB) : DB(&DB) { DB.execute("BEGIN"); }

Transaction::~Transaction() noexcept { invoke(&Transaction::rollback, this); }

Transaction::Transaction(Transaction &&Other) noexcept { swap(Other); }

Transaction &Transaction::operator=(Transaction &&Other) noexcept
{
    swap(Other);
    if (this == &Other) {
        return *this;
    }
    return *this;
}

void Transaction::swap(Transaction &Other) noexcept { std::swap(DB, Other.DB); }

void Transaction::commit()
{
    if (auto *db = std::exchange(this->DB, nullptr)) {
        db->execute("COMMIT");
    }
}

void Transaction::rollback()
{
    if (auto *db = std::exchange(this->DB, nullptr)) {
        db->execute("ROLLBACK");
    }
}

} // namespace sqldb