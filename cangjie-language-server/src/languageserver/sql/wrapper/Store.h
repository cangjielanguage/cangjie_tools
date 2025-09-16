// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "SQLiteAPI.h"
#include "Traits.h"
#include "Value.h"

#include <tuple>
#include <type_traits>
#include <utility>

namespace sqldb {
namespace impl {

template <typename Callable, std::enable_if_t<traits::is_callable<Callable>::value, int> = 0>
void store(sqlite3_stmt *S, int I, Callable &&F)
{
    using T = typename traits::function<Callable>::template argument<0>;
    std::forward<Callable>(F)(traits::value<T>(sqlite::column_value(S, I)));
}

template <typename Variable, std::enable_if_t<!traits::is_callable<Variable>::value, int> = 0>
void store(sqlite3_stmt *S, int I, Variable &V)
{
    V = traits::value<Variable>(sqlite::column_value(S, I));
}

inline void store(sqlite3_stmt *, int, decltype(std::ignore)) {}

} // namespace impl
} // namespace sqldb