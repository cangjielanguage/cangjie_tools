// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_DOCUMENTHIGHLIGHT_H
#define LSPSERVER_DOCUMENTHIGHLIGHT_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "cangjie/Lex/Token.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/AST/Symbol.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/Basic/Match.h"
#include "cangjie/Utils/FileUtil.h"
#include "cangjie/AST/Match.h"
#include "cangjie/Modules/ImportManager.h"
#include "../../logger/Logger.h"
#include "../../index/MemIndex.h"


namespace ark {
class DocumentHighlightImpl {
public:
    static std::string curFilePath;

    static void
    FindDocumentHighlights(const ArkAST &ast, std::set<DocumentHighlight> &result, Cangjie::Position pos);

    static void DealInCurPackage(const ArkAST &ast, const Position &pos, const std::vector<Symbol *> &syms,
                                 std::set<DocumentHighlight> &result, Ptr<Decl> &decl);
};
} // namespace ark

#endif
