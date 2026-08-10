// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

bool CheckerImpl::PackageDeclChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    auto& oldPkg = diff.GetOldPackage();
    auto& newPkg = diff.GetNewPackage();
    auto sameAccessMod = oldPkg.accessible == newPkg.accessible;
    CHECK(RuleKind::PACKAGE_DECL_CHANGED, sameAccessMod && oldPkg.fullPackageName == newPkg.fullPackageName);
    return checkerResult;
}