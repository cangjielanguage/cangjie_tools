// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <csignal>
#include "Commands/Heap.h"

#if defined(_WIN32)
#if !defined(SIGUSR1)
#define SIGUSR1 10
#endif
#endif

Heap::Heap() : Command("heap"), m_dump_cfg({ "/item_data.dat", SIGUSR1, false }) {}