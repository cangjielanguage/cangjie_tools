// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "SQLiteAPI.h"
#include "Traits.h"
#include "Value.h"

#include <cstddef>
#include <type_traits>
#include <utility>

namespace sqldb {
namespace impl {

template <typename Callable,
    typename... Ts,
    std::size_t I = sizeof...(Ts),
    std::enable_if_t<(I == traits::function<Callable>::arity), int> = 0>
auto invoke(sqlite3_stmt *, Callable &&F, Ts &&...Vs)
{
    return std::forward<Callable>(F)(std::forward<Ts>(Vs)...);
}

template <typename Callable,
    typename... Ts,
    std::size_t I = sizeof...(Ts),
    std::enable_if_t<(I < traits::function<Callable>::arity), int> = 0>
auto invoke(sqlite3_stmt *S, Callable &&F, Ts &&...Vs)
{
    using T = typename traits::function<Callable>::template argument<I>;
    return invoke(S, std::forward<Callable>(F), std::forward<Ts>(Vs)..., traits::value<T>(sqlite::column_value(S, I)));
}

} // namespace impl

template <typename Callable, typename... Ts>
std::exception_ptr invoke(Callable &&F, Ts &&...Args) noexcept
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        std::invoke(std::forward<Callable>(F), std::forward<Ts>(Args)...);
#ifndef NO_EXCEPTIONS
    } catch (...) {
        return std::current_exception();
    }
#endif
    return nullptr;
}

} // namespace sqldb