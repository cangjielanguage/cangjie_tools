// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGITF03_H
#define CANGJIECODECHECK_STRUCTURALRULEGITF03_H
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGITF03 : public StructuralRule {
public:
    explicit StructuralRuleGITF03(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGITF03() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
    void FindTypeDecl(Cangjie::AST::Node *node);
    void CheckInheritedTypes(const Cangjie::AST::InheritableDecl &inheritableDecl);
    void CheckParent(const std::string &childName, const Cangjie::AST::InterfaceTy *ty,
        const Cangjie::AST::InheritableDecl &inheritableDecl, std::set<Ptr<AST::InterfaceTy>> tySet = {});
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGITF03_H
