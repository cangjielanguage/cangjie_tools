// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_NORMALCOMPLETERBYPARSE_H
#define LSPSERVER_NORMALCOMPLETERBYPARSE_H

#include <cangjie/AST/ASTContext.h>
#include <string>
#include "CompletionEnv.h"
#include "CompletionImpl.h"

namespace ark {
class NormalCompleterByParse {
public:
    NormalCompleterByParse(CompletionResult &res,
        Cangjie::ImportManager *importManager,
        const Cangjie::ASTContext &ctx,
        const std::string &prefix)
        : result(res), importManager(importManager), context(&ctx), prefix(prefix)
    {
    }

    ~NormalCompleterByParse() = default;

    bool Complete(const ArkAST &input, Cangjie::Position pos);

    void CompletePackageSpec(const ArkAST &input);

    void CompleteModuleName(const std::string &curModule);

private:

    Ptr<Decl> CompleteCurrentPackages(const ArkAST &input, const Position pos, CompletionEnv &env);

    void FillingDeclsInPackage(const std::string &packageName,
                               CompletionEnv &env,
                               Ptr<const Cangjie::AST::PackageDecl> pkgDecl);

    bool DealDeclInCurrentPackage(Ptr<Decl> decl, CompletionEnv &env);

    void AddImportPkgDecl(const ArkAST &input, CompletionEnv &env);

    CompletionResult &result;

    Cangjie::ImportManager *importManager = nullptr;

    const Cangjie::ASTContext *context = nullptr;

    std::set<std::string> usedPkg = {};

    std::unordered_map<std::string, std::string> pkgAliasMap;

    std::string prefix;
};
} // namespace ark

#endif // LSPSERVER_NORMALCOMPLETERBYPARSE_H