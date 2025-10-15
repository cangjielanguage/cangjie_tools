// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_02_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_02_H
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.DCL.01 避免遮盖（shadow）
 */
class StructuralRuleGDCL02 : public StructuralRule {
public:
    explicit StructuralRuleGDCL02(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGDCL02() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void CheckReturnValType(Ptr<Cangjie::AST::Node> node);
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_02_H