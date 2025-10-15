// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGERR03_H
#define CANGJIECODECHECK_STRUCTURALRULEGERR03_H

#include <fstream>
#include <iostream>
#include <regex>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGERR03 : public StructuralRule {
public:
    explicit StructuralRuleGERR03(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGERR03() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
    void FindMemberAccess(Ptr<Cangjie::AST::Node> node);
    void CheckMemberAccessAttribute(const Cangjie::AST::MemberAccess &memberAccess);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGERR03_H
