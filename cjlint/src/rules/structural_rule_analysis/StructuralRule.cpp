// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRule.h"
#include "cangjie/Basic/Print.h"

namespace Cangjie::CodeCheck {
using namespace AST;

void StructuralRule::DoAnalysis(CJLintCompilerInstance *instance)
{
    auto packageList = instance->GetSourcePackages();

    for (auto package : packageList) {
        if (!package->TestAttr(Attribute::IMPORTED)) {
            auto &ctx = *instance->GetASTContextByPackage(package);
            MatchPattern(ctx, package);
        }
    }
}
} // namespace Cangjie::CodeCheck