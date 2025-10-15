// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGNAM05_H
#define CANGJIECODECHECK_STRUCTURALRULEGNAM05_H

#include <fstream>
#include <iostream>
#include "rules/structural_rule_analysis/StructuralRule.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
/**
 * G.NAM.05 let的全局变量和static let变量的名称采用全大写
 */
class StructuralRuleGNAM05 : public StructuralRule {
public:
    explicit StructuralRuleGNAM05(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGNAM05() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FindGlobalOrStaticVarWithLetName(Ptr<Cangjie::AST::Node> node);
    void CheckGlobalOrStaticVarWithLetName(const Cangjie::AST::VarDecl& varDecl);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURALRULEGNAM05_H