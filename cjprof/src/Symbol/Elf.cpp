// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <fstream>
#include <memory>
#include "Symbol/Elf.h"

std::vector<Symbol> Elf::ParseSymbols(const std::string &name)
{
    std::ifstream ifs(name, std::ifstream::binary);
    if (ifs.fail()) {
        return {};
    }

    ifs.seekg(0, ifs.end);
    auto size = ifs.tellg();
    if (size < 0) {
        fprintf(stderr, "error: Cannot determine '%s' file size.\n", name.c_str());
        return {};
    }
    ifs.seekg(0, ifs.beg);

    size_t buf_size = static_cast<size_t>(size);
    auto elf = std::make_unique<char[]>(buf_size);
    ifs.read(elf.get(), size);

    auto ehdr = (const Ehdr *)elf.get();
    if (!IsValidEhdr(ehdr)) {
        return {};
    }

    auto shdr = (const Shdr *)(elf.get() + ehdr->shOff);
    std::vector<Symbol> symbols;
    for (int i = 0; i < ehdr->shNum; i++) {
        if ((shdr[i].type != SYMTAB) && (shdr[i].type != DYNSYM)) {
            continue;
        }
        if (shdr[i].link >= ehdr->shNum) {
            continue;
        }
        if (shdr[i].entSize == 0) {
            continue;
        }
        auto strTab = elf.get() + shdr[shdr[i].link].offset;
        auto sym = (const Sym *)(elf.get() + shdr[i].offset);
        for (size_t j = 0; j < shdr[i].size / shdr[i].entSize; j++) {
            if ((GetSymType(sym[j]) == FUNC) && (sym[j].value != 0) && (sym[j].size != 0)) {
                symbols.push_back({sym[j].value, sym[j].size, &strTab[sym[j].name]});
            }
        }
    }

    return symbols;
}

Elf::Type Elf::GetType(const std::string &name)
{
    std::ifstream ifs(name, std::ifstream::binary);
    if (ifs.fail()) {
        return Type::NONE;
    }

    Ehdr ehdr = {{0}, 0};
    ifs.read((char *)&ehdr, sizeof(ehdr));
    return Type(ehdr.type);
}