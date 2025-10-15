// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef COMMON_RULE_H
#define COMMON_RULE_H

#include "common/CJLintCompilerInstance.h"

namespace Cangjie::CodeCheck {
enum class RuleType {
    STRUCTURAL,
    DATAFLOW,
    INFOFLOW,
    CONFIG
};

class Rule {
public:
    explicit Rule(CodeCheckDiagnosticEngine *diagEngine) : diagEngine(diagEngine) {}
    virtual ~Rule() = default;
    virtual void DoAnalysis(CJLintCompilerInstance *instance) = 0;

protected:
    CodeCheckDiagnosticEngine *diagEngine;
};
} // namespace Cangjie::CodeCheck

#endif // COMMON_RULE_H