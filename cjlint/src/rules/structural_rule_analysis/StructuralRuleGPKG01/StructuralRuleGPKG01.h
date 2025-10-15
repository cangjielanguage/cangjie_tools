// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGPKG01_H
#define CANGJIECODECHECK_STRUCTURALRULEGPKG01_H

#include <fstream>
#include <iostream>
#include <regex>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGPKG01 : public StructuralRule {
public:
    explicit StructuralRuleGPKG01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGPKG01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
    void FindImportNode(Ptr<Cangjie::AST::Node> node);
    void CheckImportItemName(const Cangjie::AST::ImportSpec &importSpec);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGPKG01_H
