// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGITF02_H
#define CANGJIECODECHECK_STRUCTURALRULEGITF02_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGITF02 : public StructuralRule {
public:
    explicit StructuralRuleGITF02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGITF02() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FindExtendDecl(Ptr<Cangjie::AST::Node> node);
    void CheckExtendMemberDecls(const Cangjie::AST::ExtendDecl &extendDecl);
    void CheckExtendMemberDeclsHelper(Ptr<Cangjie::AST::InterfaceDecl> interface, Ptr<Cangjie::AST::Decl> member,
        bool &label);
};
} // namespace Cangjie::CodeCheck

#endif // CANGJIECODECHECK_STRUCTURALRULEGITF02_H
