// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGNAM04_H
#define CANGJIECODECHECK_STRUCTURALRULEGNAM04_H

#include <fstream>
#include <iostream>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.NAM.04 函数名称应采用小驼峰命名
 */
class StructuralRuleGNAM04 : public StructuralRule {
public:
    explicit StructuralRuleGNAM04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGNAM04() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FindFuncName(Ptr<Cangjie::AST::Node> node);
    void CheckFuncName(const Cangjie::AST::FuncDecl &funcDecl);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGNAM04_H
