// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGOPR01_H
#define CANGJIECODECHECK_STRUCTURALRULEGOPR01_H

#include <fstream>
#include <iostream>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {

class StructuralRuleGOPR01 : public StructuralRule {
public:
    explicit StructuralRuleGOPR01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGOPR01() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    bool needDiag = true;
    void FindExtendDecl(Ptr<Cangjie::AST::Node> node);
    void CheckExtendDecl(Cangjie::AST::ExtendDecl& extendDecl);
    void CheckFuncDecl(Cangjie::AST::FuncDecl& funcDecl);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURALRULEGOPR01_H
