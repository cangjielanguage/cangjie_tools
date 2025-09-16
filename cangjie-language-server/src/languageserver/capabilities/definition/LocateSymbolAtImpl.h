// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_LOCATESYMBOLATIMPL_H
#define LSPSERVER_LOCATESYMBOLATIMPL_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "cangjie/Lex/Token.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/Utils/FileUtil.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/Basic/Match.h"
#include "cangjie/AST/Match.h"
#include "cangjie/AST/Symbol.h"
#include "cangjie/Modules/ImportManager.h"
#include "../../logger/Logger.h"
#include "../../common/Callbacks.h"
#include "../../CompilerCangjieProject.h"
#include "../../common/Utils.h"
#include "CrossDefinitionCangjie2C.h"

namespace ark {
struct LocatedSymbol {
    // The name of the symbol.
    std::string Name = "";
    // Where the symbol is defined.
    Location Definition;

    std::vector<message> CrossMessage;
};

class LocateSymbolAtImpl {
public:
    static std::string curFilePath;

    static bool LocateSymbolAt(const ArkAST &ast, LocatedSymbol &result, Cangjie::Position pos);

    static void CrossDefinition(std::vector<message> &CrossMessage, Ptr<Cangjie::AST::FuncDecl> funcDecl);
};
} // namespace ark

#endif // LSPSERVER_LOCATESYMBOLATIMPL_H
