// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_INDEX_SYMBOL_H
#define LSPSERVER_INDEX_SYMBOL_H

#include <string>
#include <unordered_map>
#include <vector>
#include "cangjie/AST/Node.h"

namespace ark {
namespace lsp {

using namespace Cangjie;
using SymbolID = uint64_t;
constexpr SymbolID INVALID_SYMBOL_ID = 0;

struct SymbolLocation {
    Position begin;
    Position end;
    std::string fileUri;

    bool IsZeroLoc() const
    {
        return begin.IsZero() && end.IsZero();
    }
};

enum class Modifier : uint8_t {
    UNDEFINED,
    PRIVATE,
    INTERNAL,
    PROTECTED,
    PUBLIC
};

struct Symbol {
public:
    SymbolID id;
    std::string name;
    std::string scope;
    SymbolLocation location;
    SymbolLocation declaration;
    AST::ASTKind kind;
    std::string signature;
    std::string returnType;
    bool isMemberParam{false};
    Modifier modifier{Modifier::UNDEFINED};
    bool isCjoSym{false};
    bool isDeprecated{false};
    // used in completion
    std::string insertText;
    std::string curModule;
    SymbolLocation curMacroCall;

    bool IsInvalidSym()
    {
        return id == INVALID_SYMBOL_ID;
    }
};

using SymbolSlab = std::vector<Symbol>;

struct ExtendItem {
public:
    SymbolID id;
    ark::lsp::Modifier modifier;
    std::string interfaceName;
};

using ExtendSlab = std::map<SymbolID, std::vector<ExtendItem>>;

} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_SYMBOL_H