// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_HOVERIMPL_H
#define LSPSERVER_HOVERIMPL_H

#include "../../ArkAST.h"
#include "../../CompilerCangjieProject.h"
#include "../../common/Callbacks.h"
#include "../../common/ItemResolverUtil.h"
#include "../../common/Utils.h"
#include "../../logger/Logger.h"
#include "cangjie/AST/Searcher.h"
#include "cangjie/Basic/Match.h"

namespace ark {
const std::string PKG_NAME_WHERE_APILEVEL_AT = "ohos.labels";
const std::string APILEVEL_ANNO_NAME = "APILevel";
const std::string LEVEL_IDENTGIFIER = "level";
const std::string SYSCAP_IDENTGIFIER = "syscap";

class HoverImpl {
public:
    static int FindHover(const ArkAST &ast, Hover &result, Cangjie::Position pos);

    static int GetHoverMessage(Ptr<Cangjie::AST::Decl>, Hover &, const ArkAST &ast);

private:
    static std::string curFilePath;

    static Decl *GetRealDecl(const std::vector<Ptr<Decl>> &decls);

    static void TrimSpaceAndTab(std::string &s);

    static std::string GetHoverMessageByOuterDecl(const Decl &);

    static void RemoveStar(const std::string &content, std::string &result);

    static void RemoveAboveBlank(const std::string &content, std::vector<std::string> &lines, size_t &minIndent);

    static void RemoveBlankAndStar(const std::string &content, std::string &result);

    static std::string ResolveComment(const std::string &comment, const CommentKind kind);

    static std::string GetDeclApiKey(const Ptr<Decl> &decl);

    static bool IsAnnoAPILevel(Ptr<Annotation> anno, Ptr<ASTContext> ctx);

    static std::string GetDeclApiLevelAnnoInfo(Decl &decl, const ArkAST &ast);
};
} // namespace ark

#endif // LSPSERVER_HOVERIMPL_H
