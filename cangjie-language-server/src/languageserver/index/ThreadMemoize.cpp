// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ThreadMemoize.h"
#include <atomic>

namespace ark {
namespace lsp {

enum class ShutdownState {
    None,
    Pending,
    Requested
};
std::atomic<ShutdownState> shutdownState = {ShutdownState::None};

bool ShutdownRequested() { return shutdownState.load() == ShutdownState::Requested; }

bool ShutdownPending() { return shutdownState.load() >= ShutdownState::Pending; }

} // namespace lsp
} // namespace ark
