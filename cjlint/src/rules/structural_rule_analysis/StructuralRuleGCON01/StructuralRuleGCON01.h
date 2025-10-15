// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_01_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_01_H

#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.CON.01 禁止将系统内部使用的锁对象暴露给不可信代码
 */
class StructuralRuleGCON01 : public StructuralRule {
public:
    explicit StructuralRuleGCON01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGCON01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void ClassModifierChecker(const Cangjie::AST::VarDecl &varDecl);
    void DefaultPackageChecker(const std::set<Cangjie::AST::Modifier> modifiers, const Cangjie::Position start,
        const Cangjie::Position end);
    void PackageGlobalVarChecker(const std::set<Cangjie::AST::Modifier> modifiers, const Cangjie::Position start,
        const Cangjie::Position end);
    void SynchronizedObjectFinder(Ptr<Cangjie::AST::Node> node);
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_01_H