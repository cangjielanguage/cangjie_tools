// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_TYP_03_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_TYP_03_H

#include "cangjie/AST/Match.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.TYP.03 判断变量是否为 NaN 时必须使用 isNaN() 方法
 */
class StructuralRuleGTYP03 : public StructuralRule {
public:
    explicit StructuralRuleGTYP03(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGTYP03() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FloatBinaryFinder(Ptr<Cangjie::AST::Node> node);
    void CheckBinaryExpr(const Cangjie::AST::BinaryExpr& binaryExpr);
    void AnalyzeBinaryExpr(const Ptr<Cangjie::AST::Expr> expr1, const Ptr<Cangjie::AST::Expr>& expr2);
};
} // namespace Cangjie::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_TYP_03_H
