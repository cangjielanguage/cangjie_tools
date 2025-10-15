// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEP03_H
#define CANGJIECODECHECK_STRUCTURALRULEP03_H

#include "nlohmann/json.hpp"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleP03 : public StructuralRule {
public:
    explicit StructuralRuleP03(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleP03() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;

    void MemberAccessProcessor(Ptr<Cangjie::AST::Node> node);
    void RefExprProcessor(Ptr<Cangjie::AST::Node> node);
    void FindCallExpr(Ptr<Cangjie::AST::Node> node);
    bool IsSecurityCheckCallExprHelper(Ptr<Cangjie::AST::Decl> target);
    bool IsSecurityCheckCallExpr(Ptr<Cangjie::AST::CallExpr> pCallExpr);
    void FindFuncDecl(Ptr<Cangjie::AST::FuncArg> funcArg);
};
}

#endif
