// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CjoLoader/CjoLoader.h"

const std::string CjoLoader::PACKAGENAME = "PACKAGE";

bool CjoLoader::Load()
{
    Cangjie::AST::Package ipkg{};
    auto pkgs = std::vector<Ptr<Cangjie::AST::Package>>{&ipkg};
    if (!ci.importManager->BuildIndex(modulesPath, loadInvocation.globalOptions, pkgs)) {
        return false;
    }
    // load twice the package to get the package name
    package = ci.importManager->LoadPackageFromCjo(CjoLoader::PACKAGENAME, cjoPath);
    if (!package) {
        // cjo file is not exist.
        return false;
    }
    if (package->fullPackageName != "default") {
        package = ci.importManager->LoadPackageFromCjo(package->fullPackageName, cjoPath);
    }
    // We need to run PerformSema so that the typeManager gets populated. This is important to get information about
    // type hierarchy for instance.
    ci.PerformSema();
    return true;
}

const Cangjie::AST::Package* CjoLoader::GetPackage() const
{
    return package;
}

Cangjie::SourceManager& CjoLoader::GetSourceManager()
{
    return ci.GetSourceManager();
}

Cangjie::TypeManager& CjoLoader::GetTypeManager()
{
    return *ci.typeManager;
}
