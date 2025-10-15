// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGVAR03_H
#define CANGJIECODECHECK_STRUCTURALRULEGVAR03_H

#include <fstream>
#include <iostream>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.VAR.03 避免使用全局变量
 */
class StructuralRuleGVAR03 : public StructuralRule {
public:
    explicit StructuralRuleGVAR03(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGVAR03() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void CheckVarDecl(const Cangjie::AST::VarDecl &varDecl);
    void FindGlobalVar(Ptr<Cangjie::AST::Node> node);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGVAR03_H
