// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGNAM02_H
#define CANGJIECODECHECK_STRUCTURALRULEGNAM02_H

#include <fstream>
#include <iostream>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.NAM.02 源文件名采用全小写加下划线风格
 */
class StructuralRuleGNAM02 : public StructuralRule {
public:
    explicit StructuralRuleGNAM02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGNAM02() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FindFileName(Ptr<Cangjie::AST::Node> node);
    void CheckNameRule(Cangjie::AST::File &file);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGNAM02_H
