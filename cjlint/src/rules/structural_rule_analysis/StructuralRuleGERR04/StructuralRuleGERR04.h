// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGERR04_H
#define CANGJIECODECHECK_STRUCTURALRULEGERR04_H

#include <fstream>
#include <iostream>
#include <regex>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGERR04 : public StructuralRule {
public:
    explicit StructuralRuleGERR04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGERR04() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
    void FindFinallyBlock(Ptr<Cangjie::AST::Node> node);
    void CheckNode(Ptr<Cangjie::AST::Node> node, bool inChildScope = false);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGERR04_H
