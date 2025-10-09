// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_DOCUMENTSYMBOLIMPL_H
#define LSPSERVER_DOCUMENTSYMBOLIMPL_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"

namespace ark {
class DocumentSymbolImpl {
public:
    static void FindDocumentSymbols(const ArkAST &ast, std::vector<DocumentSymbol> &result);

    static std::map<Cangjie::AST::Attribute, std::string> GetSupportAttribute();

private:
    static std::map<Cangjie::AST::Attribute, std::string> supportAttribute;

    static std::unordered_set<Cangjie::AST::ASTKind> supportSymbolKind;
};
}


#endif // LSPSERVER_DOCUMENTSYMBOLIMPL_H
