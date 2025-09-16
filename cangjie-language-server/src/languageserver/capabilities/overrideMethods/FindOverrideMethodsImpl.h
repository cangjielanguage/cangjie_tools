// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_FINDOVERRIDEMETHODSIMPL_H
#define LSPSERVER_FINDOVERRIDEMETHODSIMPL_H

#include "../../ArkAST.h"

namespace ark {

struct OverrideMethodInfo {
    bool deprecated{false};
    bool isProp{false};
    std::string signatureWithRet;
    std::string insertText;
};

struct OverrideMethodsItem {
    std::string package;
    std::string kind;
    std::string identifier;
    std::vector<OverrideMethodInfo> overrideMethodInfos;
};

struct FindOverrideMethodResult {
    std::vector<OverrideMethodsItem> overrideMethods;
};

using OverridableFuncAndPropMap =
    std::unordered_map<Ptr<InheritableDecl>, std::pair<std::vector<FuncDecl*>, std::vector<PropDecl*>>>;

class FindOverrideMethodsImpl {
    static std::string fullPkgName;
    static std::string curFilePath;
    static std::unordered_map<Ptr<InheritableDecl>, std::unordered_map<std::string, std::string>> genericReplaceMap;

public:
    static void FindOverrideMethods(const ArkAST &ast, FindOverrideMethodResult &result, Cangjie::Position pos,
                                    bool isExtend);
    static void GetImplementedMethodsAndProps(const ArkAST &ast, Ptr<InheritableDecl> decl,
                                              std::vector<FuncDecl*>& implementedMethods,
                                              std::vector<PropDecl*>& implementedDecls);
    static void GetOverridableMethodsAndPropsMap(Ptr<InheritableDecl> decl,
                                                 OverridableFuncAndPropMap& overrideableMethodsAndPropMap,
                                                 const std::vector<FuncDecl*>& implementedMethods,
                                                 const std::vector<PropDecl*>& implementedProps);
    static void AddFuncItemsToResult(const Ptr<Decl>& decl, const Ptr<InheritableDecl>& owner,
                                     OverrideMethodsItem& item, const std::vector<FuncDecl*>& overridableMethods,
                                     const std::vector<Ptr<ClassLikeDecl>>& canSuperCall);

    static void AddPropItemsToResult(const Ptr<Decl> decl, const Ptr<InheritableDecl> owner,
                                     OverrideMethodsItem& item, const std::vector<PropDecl*>& overridableProps,
                                     const std::vector<Ptr<ClassLikeDecl>>& canSuperCall);
};
} // namespace ark
#endif // LSPSERVER_FINDOVERRIDEMETHODSIMPL_H
