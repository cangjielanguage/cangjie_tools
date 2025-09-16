// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_OVERRIDECOMPLETER_H
#define LSPSERVER_OVERRIDECOMPLETER_H

#include <vector>
#include "cangjie/AST/Node.h"
#include "../../common/ItemResolverUtil.h"
#include "../overrideMethods/FindOverrideMethodsUtils.h"
#include "CompletionImpl.h"
#include "cangjie/Basic/Position.h"

namespace ark {
using namespace Cangjie::AST;

class OverrideCompleter {
public:
    OverrideCompleter() = default;

    OverrideCompleter(Ptr<Cangjie::AST::Decl> decl, const std::string& prefix)
        : topLevelDecl(decl), prefix(prefix) {}

    void FindOvrrideFunction();

    std::vector<CodeCompletion> ExportItems();

    /**
     * Check function start postion, modifer.
     */
    bool SetCompletionConfig(Ptr<Decl> decl, const Position& pos);

private:

    void ResolveDeclModifiers(const Ptr<Decl> &decl);

    void CompleteFuncDecl(Ptr<FuncDecl> decl,
        const std::optional<std::unordered_map<std::string, std::string>>& replace);

    std::string GetTextString(const std::vector<std::string>& strs);

    std::string GetModifierString(const std::vector<std::string>& strs,
                                  const std::unordered_set<std::string>& filter);

    void FilterModifiers();

    std::vector<Ptr<ClassLikeDecl>> GetSuperCallDecls(Ptr<Decl> decl);

    void ExtractReplace(Ptr<Decl> decl);

    bool CheckConfilctModifer(Ptr<Decl> decl);

    bool CheckRepeated(Ptr<FuncDecl> decl);

    std::string GetFuncLabel(const FuncDetail& funcDetail);

    Ptr<Cangjie::AST::Decl> topLevelDecl = nullptr;

    std::map<Ptr<InheritableDecl>, std::vector<Ptr<FuncDecl>>> funcMap{};

    std::string prefix{};

    std::vector<FuncDetail> functionDetails;

    Position additionalPos;

    std::optional<Attribute> modifier;

    std::unordered_set<std::string> modifierSet;

    std::unordered_set<std::string> curModifierSet;

    bool hasFuncWord = true; // if decl has a func keyword

    std::vector<Ptr<FuncDecl>> implementedMethods;

    std::unordered_map<Ptr<InheritableDecl>, std::unordered_map<std::string, std::string>> genericTypeMap;
};

} // namespace ark

#endif // LSPSERVER_OVERRIDECOMPLETER_H