// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGITF04_H
#define CANGJIECODECHECK_STRUCTURALRULEGITF04_H

#include "rules/structural_rule_analysis/StructuralRule.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGITF04 : public StructuralRule {
public:
    explicit StructuralRuleGITF04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGITF04() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
    void FindFuncDecl(Cangjie::AST::Node *node);
    void CheckFuncDeclParams(const Cangjie::AST::FuncDecl &funcDecl);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGITF04_H
