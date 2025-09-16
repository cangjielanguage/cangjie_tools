// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include <type_traits>
#include <utility>

namespace sqldb {

template <typename Callable>
class [[nodiscard]] ScopeExit {
    static_assert(std::is_nothrow_invocable_v<std::decay_t<Callable>>);

public:
    ScopeExit(Callable &&F) noexcept : F(std::forward<Callable>(F)) {}

    ScopeExit(const ScopeExit &) = delete;
    void operator=(const ScopeExit &) = delete;

    ~ScopeExit() noexcept { F(); }

private:
    std::decay_t<Callable> F;
};

} // namespace sqldb