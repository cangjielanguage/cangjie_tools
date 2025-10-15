// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGNAM06_H
#define CANGJIECODECHECK_STRUCTURALRULEGNAM06_H

#include <fstream>
#include <iostream>
#include "rules/structural_rule_analysis/StructuralRule.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
/**
 * G.NAM.06 变量的名称采用小驼峰
 */
class StructuralRuleGNAM06 : public StructuralRule {
public:
    explicit StructuralRuleGNAM06(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGNAM06() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void CheckCamelCaseFormat(Ptr<Cangjie::AST::Node> node);
    void PrintDiagnoseInfo(SrcIdentifier& identifier, std::string pattern, CodeCheckDiagKind kind);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURALRULEGNAM06_H