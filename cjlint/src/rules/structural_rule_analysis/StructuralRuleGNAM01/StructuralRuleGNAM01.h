// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGNAM01_H
#define CANGJIECODECHECK_STRUCTURALRULEGNAM01_H

#include <regex>
#include "rules/structural_rule_analysis/StructuralRule.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
/**
 * G.NAM.01 包名采用全小写单词，允许包含数字和下划线
 */

class StructuralRuleGNAM01 : public StructuralRule {
public:
    explicit StructuralRuleGNAM01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGNAM01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FilePackageCheckingFunction(Ptr<Cangjie::AST::Node> node);
    void FileDeclHandler(const Cangjie::AST::File &file);
};
} // namespace Cangjie::CodeCheck

#endif // CANGJIECODECHECK_STRUCTURALRULEGNAM01_H
