// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>

struct Symbol {
    uint64_t addr;
    uint64_t size;
    std::string name;
    bool operator<(const Symbol &rhs) const
    {
        return (addr + size) <= rhs.addr;
    };
};

#endif // ELF_H