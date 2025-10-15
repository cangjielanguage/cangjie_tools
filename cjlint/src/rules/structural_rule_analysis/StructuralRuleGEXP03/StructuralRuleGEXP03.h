// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_03_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_03_H

#include "cangjie/AST/Match.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.EXP.04：&& 、 ||、? 和 ?? 操作符的右侧操作数不要包含副作用
 */
class StructuralRuleGEXP03 : public StructuralRule {
public:
    explicit StructuralRuleGEXP03(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGEXP03() override = default;
protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    Ptr<Cangjie::AST::Node> originNode = nullptr;
    std::set<Cangjie::Position> sideEffectSet;
    std::vector<Cangjie::Position> callStack;
    std::set<Cangjie::Position> checkedFunc;
    void RightOperandFinder(Ptr<Cangjie::AST::Node> node);
    std::string GetOperandName(const Cangjie::AST::AssignExpr &assignExpr);
    void SideEffectChecker(Ptr<Cangjie::AST::Node> node);
    void SideEffectFuncFinder(Ptr<Cangjie::AST::Node> node);
    void SideEffectExprFinder(Ptr<Cangjie::AST::Node> node, const Cangjie::Position position);
    void GlobalRefChecker(Ptr<Cangjie::AST::Node> node, const Cangjie::Position originFuncPosition,
        const Cangjie::Position refFuncPosition);
    void ClassRefChecker(Ptr<Cangjie::AST::Node> node, const Cangjie::Position originFuncPosition,
        const Cangjie::Position refFuncPosition);
    bool AddRefFuncInfo(Ptr<const Cangjie::AST::Node> node, const Cangjie::Position originFuncPosition,
        const Cangjie::Position refFuncPosition);
    void ProcessRefFuncInfo(const AST::RefExpr &refExpr, const Cangjie::Position originFuncPosition);
    void FindMemberAccess(const AST::MemberAccess &memberAccess);
    void ClassRefCheckerDetail(const AST::RefExpr &refExpr, const Cangjie::Position originFuncPosition);
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_03_H
