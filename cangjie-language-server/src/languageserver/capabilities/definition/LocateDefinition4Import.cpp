// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "LocateDefinition4Import.h"

namespace {
using namespace ark;
std::string GetPackagePrefixWithPaths(const std::vector<std::string> &prefixPaths)
{
    std::stringstream ss;
    for (const auto &prefix : prefixPaths) {
        ss << prefix << ".";
    }
    return ss.str().substr(0, ss.str().size() - 1);
};    
    
Ptr<Decl> ProcSingleImport(
    const ArkAST &ast, File *fileNode,
    ImportContent &importContent,
    std::string packagePrefix = "")
{
    const auto &importManager = ast.packageInstance->importManager;
    const auto srcPkgName = fileNode->curPackage->fullPackageName;
    if (packagePrefix.empty()) {
        packagePrefix = GetPackagePrefixWithPaths(importContent.prefixPaths);
    }

    if (const auto targetPkg = importManager.GetPackageDecl(packagePrefix)) {
        const auto members = importManager.GetPackageMembers(srcPkgName, targetPkg->fullPackageName);
        for (const auto &memberDecl : members) {
            if (const auto declPtr = dynamic_cast<const Decl *>(memberDecl.get());
                declPtr == nullptr || declPtr->identifier != importContent.identifier)
                continue;
            return memberDecl;
        }
    }
    return Ptr<Decl>();
};
}

namespace ark {
Ptr<Decl> LocateDefinition4Import::getDecl(const ArkAST &ast, Position pos, File *fileNode)
{
    for (auto &fileImport : fileNode->imports) {
        auto importContent = fileImport.get()->content;
        if (fileImport.get()->IsImportSingle() &&
            pos <= importContent.identifier.End() &&
            pos >= importContent.identifier.Begin()) {
            return ProcSingleImport(ast, fileNode, importContent);
        }
        if (fileImport.get()->IsImportAlias() &&
            (pos <= importContent.identifier.End() && pos >= importContent.identifier.Begin() ||
             pos <= importContent.aliasName.End() && pos >= importContent.aliasName.Begin())) {
            return ProcSingleImport(ast, fileNode, importContent);
        }
        if (fileImport.get()->IsImportMulti()) {
            auto packagePrefix = GetPackagePrefixWithPaths(importContent.prefixPaths);
            for (auto &item : importContent.items) {
                if (pos > item.identifier.End() || pos < item.identifier.Begin())
                    continue;
                return ProcSingleImport(ast, fileNode, item, packagePrefix);
            }
        }
    }
    return Ptr<Decl>();
}

void LocateDefinition4Import::getImportDecl(const std::vector<Symbol *> &syms,
                                                  const ArkAST &ast,
                                                  Position pos,
                                                  Ptr<Decl> &decl)
{
    File *fileNode;
    for (const auto sym : syms) {
        if (sym && sym->astKind == ASTKind::FILE) {
            if ((fileNode = dynamic_cast<File *>(sym->node.get())) != nullptr) {
                decl = getDecl(ast, pos, fileNode);
                break;
            }
        }
    }
}
}