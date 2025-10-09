// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_BASICHELPER_H
#define LSPSERVER_BASICHELPER_H

#ifdef _WIN32
#include "windows.h"
#elif __linux__
#include "unistd.h"
#include "sys/sysinfo.h"
#endif

#include <functional>
#include <cstdint>
#include "../../json-rpc/Protocol.h"
#include "cangjie/Basic/Position.h"

namespace ark {
template<typename T>
using Callback = std::function<void(T)>;

std::int32_t GetOffsetFromPosition(const std::string &, Cangjie::Position);
} // namespace ark

#endif // LSPSERVER_BASICHELPER_H
