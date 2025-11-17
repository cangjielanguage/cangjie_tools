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
class HoverImpl {
public:
    static int FindHover(const ArkAST& ast, Hover& result, Cangjie::Position pos);

    static std::string GetDeclCommentByIndex(const std::vector<Cangjie::Token>&,
                                             const unsigned int, const int, const int);

    static std::string GetDeclCommentOfBack(const std::vector<Cangjie::Token>&, const int);

    static std::string GetDeclCommentOfAbove(const std::vector<Cangjie::Token>&, const unsigned int, bool&, const int);

    static int GetHoverMessage(Ptr<Cangjie::AST::Decl>, Hover &, const ArkAST &ast);

private:
    static std::string curFilePath;

    static std::vector<std::string> StringSplit(const std::string& src, const std::string& separateCharacter);

    static void TrimSpaceAndTab(std::string& s);

    static void TrimBlankLines(std::vector<std::string>&);

    static void GetEffectiveContent(std::string& content, bool isHeadOrTail);

    static void ResolveDocMapAndDocKey(
        std::unordered_map<std::string, std::vector<std::string>>&, std::vector<std::string>&, std::string);

    static std::string GetHoverMessageForBlockComment(const std::vector<std::string>&);

    static std::string GetEffectiveDocComment(std::vector<std::string>&, const std::string&);

    static std::string GetHoverMessageForDocComment(const std::vector<std::string>&);

    static std::string GetHoverMessageByOuterDecl(const Decl&);

    static std::string ResolveComment(const std::string& comment, const CommentKind kind);

    static std::string GetDeclApiKey(const Ptr<Decl> &decl);

    static bool ValidTag(const char ch);

    /**
     * get var decl for real modifier when ast kind is func param
     *
     * @param decls decls
     * @return var decl
     */
    static Decl* GetRealDecl(const std::vector<Ptr<Decl>> &decls);
};
} // namespace ark

#endif // LSPSERVER_HOVERIMPL_H
