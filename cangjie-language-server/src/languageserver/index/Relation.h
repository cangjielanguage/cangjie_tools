// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_INDEX_RELATION_H
#define LSPSERVER_INDEX_RELATION_H

#include <cstdint>
#include "Symbol.h"

namespace ark {
namespace lsp {
enum class RelationKind : uint8_t {
    BASE_OF,    // Type inheritance relation. eg: A <: B, B is base of A.
    RIDDEND_BY, // Member relation. eg: open A.f1, override B.f1, A.f1 ridden by B.f1.
    EXTEND,     // Type extend relation. eg: extend A <: I, A extend I.
    CALLED_BY,
    CONTAINED_BY,
    OVERRIDES,
};

struct Relation {
    SymbolID subject;
    RelationKind predicate;
    SymbolID object;
};

using RelationSlab = std::vector<Relation>;

} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_RELATION_H