// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGVAR02_H
#define CANGJIECODECHECK_STRUCTURALRULEGVAR02_H

#include <fstream>
#include <iostream>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.VAR.02 保持变量的作用域尽可能小
 */
class StructuralRuleGVAR02 : public StructuralRule {
public:
    explicit StructuralRuleGVAR02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGVAR02() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void CollectVarDeclMap(Ptr<Cangjie::AST::Node> node, size_t level = 0);
    void CheckVarDeclMap();
    std::map<Ptr<Cangjie::AST::VarDecl>, std::set<std::string>> varDeclMap;
    std::set<Ptr<Cangjie::AST::VarDecl>> varDeclSet;
    size_t inSpawnLevel = 0;
    std::string scopeName = "";
    Position position = {0, 0, 0};
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGVAR02_H
