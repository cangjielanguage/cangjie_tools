// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_INDEX_CALLRELATION_H
#define LSPSERVER_INDEX_CALLRELATION_H

#include <string>
#include <unordered_map>
#include <vector>
#include "Symbol.h"

namespace ark {
namespace lsp {

enum class CallRelationKind : uint8_t {
    CALLED_BY,
    CALLS,
    CONTAINED_BY,
    CONTAINS
};

// Called by -> Calls, Calls -> Called by
// Contained by -> Contains, Contains -> Contained by
inline CallRelationKind InvertCallRelationKind(CallRelationKind kind)
{
    switch (kind) {
        case CallRelationKind::CALLED_BY:
            return CallRelationKind::CALLS;
        case CallRelationKind::CALLS:
            return CallRelationKind::CALLED_BY;
        case CallRelationKind::CONTAINED_BY:
            return CallRelationKind::CONTAINS;
        case CallRelationKind::CONTAINS:
            return CallRelationKind::CONTAINED_BY;
    }
}

/// Represents a relation between two symbols.
/// For an example "B calls A" may be represented
/// as { Subject = A, Predicate = CALLED_BY, Object = B, Location }.
struct CallRelation {
    SymbolID subject;
    CallRelationKind predicate;
    SymbolID object;
    SymbolLocation location;
};
} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_CALLRELATION_H
