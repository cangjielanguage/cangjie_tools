// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGEXP07_H
#define CANGJIECODECHECK_STRUCTURALRULEGEXP07_H

#include "cangjie/AST/Match.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGEXP07 : public StructuralRule {
public:
    explicit StructuralRuleGEXP07(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGEXP07() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void BinaryDesugar(const Cangjie::AST::BinaryExpr &binaryExpr);
    void BinaryNotDesugar(const Cangjie::AST::BinaryExpr &binaryExpr);

    void FindBinaryExpr(Cangjie::AST::Node *node);
    void CheckBinaryExpr(const Cangjie::AST::BinaryExpr &binaryExpr);
    void ReferenceExprAnalysis(Ptr<AST::Expr> expr, Ptr<AST::VarDecl> rightVarDecl);
    void VarDeclDiagnose(Ptr<AST::Decl> decl, Position& start, Position& end, Ptr<AST::VarDecl> rightVarDecl);
    bool ExcludeInterval(const Cangjie::AST::BinaryExpr &binaryExpr);
    bool ExcludeIntervalHelper(const Cangjie::AST::BinaryExpr &binaryExpr);
    bool ExcludeIntervalCoverToVarDecl(Cangjie::AST::VarDecl *lv, Cangjie::AST::VarDecl *rv,
        Ptr<Cangjie::AST::BinaryExpr> lb, Ptr<Cangjie::AST::BinaryExpr> rb);
    bool IsTuple(const AST::BinaryExpr &binaryExpr);
    void ExprCoverToVarDecl(Ptr<AST::Expr> expr, Ptr<AST::VarDecl> &varDecl);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGEXP07_H
