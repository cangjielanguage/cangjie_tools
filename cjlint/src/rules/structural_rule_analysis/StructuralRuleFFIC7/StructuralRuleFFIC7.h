// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEFFIC7_H
#define CANGJIECODECHECK_STRUCTURALRULEFFIC7_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
class StructuralRuleFFIC7 : public StructuralRule {
public:
    explicit StructuralRuleFFIC7(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleFFIC7() override = default;
protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
private:
    void CheckPointerExpr(Ptr<Cangjie::AST::Node> node);
};
}

#endif
