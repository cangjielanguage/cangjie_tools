// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGEXP06_H
#define CANGJIECODECHECK_STRUCTURALRULEGEXP06_H

#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGEXP06 : public StructuralRule {
public:
    explicit StructuralRuleGEXP06(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGEXP06() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
    void FindBinaryExpr(Ptr<Cangjie::AST::Node> node);
    void CheckBinaryExpr(const Cangjie::AST::BinaryExpr &binaryExpr);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGEXP06_H
