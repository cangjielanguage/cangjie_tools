// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGOPR02_H
#define CANGJIECODECHECK_STRUCTURALRULEGOPR02_H

#include <fstream>
#include <iostream>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.OPR.02 尽量避免在枚举类型内定义`()`操作符重载函数
 */
class StructuralRuleGOPR02 : public StructuralRule {
public:
    explicit StructuralRuleGOPR02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGOPR02() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FindEnum(Ptr<Cangjie::AST::Node> node);
    void CheckParenthesesOpertorInEnum(Cangjie::AST::EnumDecl &enumDecl);
    void CheckParenthesesOpertorInExtend(Cangjie::AST::ExtendDecl &extendDecl);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGOPR02_H
