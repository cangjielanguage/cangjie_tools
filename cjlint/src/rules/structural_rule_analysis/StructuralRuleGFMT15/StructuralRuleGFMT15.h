// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_15_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_15_H

#include "cangjie/AST/Match.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.FMT.15 禁止省略浮点数小数点前的 0
 */
class StructuralRuleGFMT15 : public StructuralRule {
public:
    explicit StructuralRuleGFMT15(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGFMT15() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void CheckLitConstExpr(const Cangjie::AST::LitConstExpr& litConstExpr);
    void FindLitConstExpr(Ptr<Cangjie::AST::Node> node);
};
} // namespace Cangjie::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_15_H
