// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_ENU_01_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_ENU_01_H

#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.ENU.01：避免枚举的构造成员与顶层元素同名
 */
class StructuralRuleGENU01 : public StructuralRule {
public:
    explicit StructuralRuleGENU01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGENU01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    using PositionPair = std::pair<Position, Position>;
    std::set<std::pair<std::string, PositionPair>> enumSet;
    void DuplicateNameFinderHelper(Ptr<Cangjie::AST::Node> node);
    void DiagnosticsFunc(const Cangjie::AST::Decl &decl);
    void DuplicateNameFinder(Ptr<Cangjie::AST::Node> node);
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_ENU_01_H
