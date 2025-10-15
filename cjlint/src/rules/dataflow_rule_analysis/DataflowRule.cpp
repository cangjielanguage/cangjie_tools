// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DataflowRule.h"

using namespace Cangjie::CHIR;

using namespace Cangjie::CodeCheck;

void DataflowRule::DoAnalysis(CJLintCompilerInstance *instance)
{
    // Traverse and inspect CHIR packages
    analysisWrapper = const_cast<AnalysisWrapper*>(&instance->GetConstAnalysisWrapper());
    for (auto chirPkg : instance->GetAllCHIRPackages()) {
        CheckBasedOnCHIR(*chirPkg);
    }
}
