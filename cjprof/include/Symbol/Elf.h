// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef ELF_H
#define ELF_H

#include <cstdint>
#include <string>
#include <vector>
#include "Symbol/Symbol.h"

class Elf {
public:
    enum class Type {
        NONE = 0,
        REL = 1,
        EXEC = 2,
        DYN = 3,
        CORE = 4
    };

    static std::vector<Symbol> ParseSymbols(const std::string &name);
    static Type GetType(const std::string &name);

private:
    enum SecType {
        SYMTAB = 2,
        STRTAB = 3,
        DYNSYM = 11,
    };

    enum SymType {
        FUNC = 2
    };

    /* 文件头数据结构 */
    struct Ehdr {
        uint8_t ident[16];
        uint16_t type;
        uint16_t machine;
        uint32_t version;
        uint64_t entry;
        uint64_t phOff;
        uint64_t shOff;
        uint32_t flags;
        uint16_t ehSize;
        uint16_t phEntSize;
        uint16_t phNum;
        uint16_t shEntSize;
        uint16_t shNum;
        uint16_t shStrIdx;
    };

    /* 节区头数据结构 */
    struct Shdr {
        uint32_t name;
        uint32_t type;
        uint64_t flags;
        uint64_t addr;
        uint64_t offset;
        uint64_t size;
        uint32_t link;
        uint32_t info;
        uint64_t addrAlign;
        uint64_t entSize;
    };

    /* 符号表项格式定义 */
    struct Sym {
        uint32_t name;
        uint8_t info;
        uint8_t other;
        uint16_t shIdx;
        uint64_t value;
        uint64_t size;
    };

    static bool IsValidEhdr(const Ehdr *ehdr)
    {
        if (ehdr == nullptr) {
            return false;
        }
        return ((ehdr->ident[0] == 0x7f) && (ehdr->ident[1] == 'E') &&
            (ehdr->ident[2] == 'L') && (ehdr->ident[3] == 'F'));
    }

    static SymType GetSymType(const Sym &sym)
    {
        return SymType(sym.info & 0xfU);
    }
};

#endif // ELF_H