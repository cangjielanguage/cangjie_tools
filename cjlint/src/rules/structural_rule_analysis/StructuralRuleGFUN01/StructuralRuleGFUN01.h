// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_01_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_01_H

#include "rules/structural_rule_analysis/StructuralRule.h"
#include "common/CommonFunc.h"

namespace Cangjie::CodeCheck {
/**
 * 仓颉编程语言通用编程规范的0.1版本
 * G.FUN.01 函数功能要单一
 */
class StructuralRuleGFUN01 : public StructuralRule {
public:
    explicit StructuralRuleGFUN01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGFUN01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FuncAndCodeControlBlockFinder(Ptr<Cangjie::AST::Node> node);
    void CheckFuncHelper(const AST::FuncDecl &funcDecl);
    void CheckLambdaExprHelper(const AST::LambdaExpr &lambdaExpr);
    void CheckControlExpr(const  AST::Expr &expr, size_t &depth);
    void CheckControlExprHelper(std::vector<OwnedPtr<AST::Node>>& body, size_t& depth);
    void CheckControlBlock(const AST::Expr& expr, AST::ASTKind astKind);
    void CheckIfExprHelper(const AST::Expr& expr, size_t& depth);
    void CheckWhileExpr(const AST::Expr& expr, size_t& depth);
    void CheckDoWhileExpr(const AST::Expr& expr, size_t& depth);
    void CheckMatchExpr(const AST::Expr& expr, size_t& depth);
    void CheckForInExpr(const AST::Expr& expr, size_t& depth);
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_01_H