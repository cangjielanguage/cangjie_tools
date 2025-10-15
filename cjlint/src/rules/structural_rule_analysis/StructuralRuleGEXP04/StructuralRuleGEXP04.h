// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_04_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_04_H

#include "cangjie/AST/Match.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * 仓颉编程语言通用编程规范
 * G.EXP.04：尽量避免副作用发生依赖于操作符的求值顺序
 */
class StructuralRuleGEXP04 : public StructuralRule {
public:
    explicit StructuralRuleGEXP04(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGEXP04() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void CheckSideEffect(Ptr<Cangjie::AST::Node> node);
    bool HasSideEffectInFunc(Ptr<Cangjie::AST::FuncDecl> funcDecl);
    bool CheckSideEffectInExpr(Ptr<AST::Expr> expr);
    std::unordered_map<Ptr<AST::FuncDecl>, bool> funcHasCheckedMap;
    Ptr<AST::FuncDecl> funcWithSideEffect;
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_04_H
