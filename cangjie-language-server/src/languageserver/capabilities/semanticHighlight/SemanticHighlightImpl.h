// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_SEMANTICHIGHLIGHT_H
#define LSPSERVER_SEMANTICHIGHLIGHT_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "cangjie/Lex/Token.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/AST/Match.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Basic/Match.h"
#include "cangjie/AST/Symbol.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/Utils/FileUtil.h"
#include "cangjie/Modules/ImportManager.h"
#include "../../logger/Logger.h"

namespace ark {
bool operator==(const SemanticHighlightToken &, const SemanticHighlightToken &);

class SemanticHighlightImpl {
public:
    static void FindHighlightsTokens(const ArkAST &ast, std::vector<SemanticHighlightToken> &result,
                                     unsigned int fileID);
    static bool NodeValid(const Ptr<Node> node, unsigned int fileID, const std::string &name);
    static bool NeedHightlight(const ArkAST &ast, const Ptr<Node> &node);
};
} // namespace ark

#endif // LSPSERVER_SEMANTICHIGHLIGHT_H
