// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGITF01_H
#define CANGJIECODECHECK_STRUCTURALRULEGITF01_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGITF01 : public StructuralRule {
public:
    explicit StructuralRuleGITF01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGITF01() override = default;

    void DoAnalysis(CJLintCompilerInstance* instance) override;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;
    void MatchPattern(Ptr<Cangjie::AST::Node> node, TypeManager* typeManager);

private:
    void HasModifiedVar(Ptr<AST::AssignExpr> pAssignExpr, std::unordered_set<Ptr<AST::Decl>>& varDecls,
        const Ptr<AST::Decl> parentFuncDecl);
    void AnalysisFuncDecl(Ptr<AST::FuncDecl> pFuncDecl, std::unordered_set<Ptr<AST::Decl>>& varDecls,
        const Ptr<AST::Decl> parentFuncDecl);
    void CoverDeclToFuncDecl(TypeManager* typeManager, Ptr<AST::Decl> pDecl,
        std::unordered_set<Ptr<AST::Decl>>& varDecls, std::unordered_set<Ptr<AST::Decl>>& funcDecls);
    void AnalysisFuncDeclsOfExtendDecl(TypeManager* typeManager, Ptr<AST::ExtendDecl> pExtendDecl,
        std::unordered_set<Ptr<AST::Decl>>& varDecls, std::unordered_set<Ptr<AST::Decl>>& funcDecls);
    void AnalysisFuncDeclsOfStructDecl(TypeManager* typeManager, Ptr<AST::StructDecl> pStructDecl,
        std::unordered_set<Ptr<AST::Decl>>& varDecls, std::unordered_set<Ptr<AST::Decl>>& funcDecls);
    void CollectVarDeclsOfStructDecl(Ptr<AST::StructDecl> pStructDecl, std::unordered_set<Ptr<AST::Decl>>& varDecls);
    void AnalysisInterfaceDecl(const AST::InterfaceDecl& interfaceDecl, TypeManager* typeManager);
    void FindInterfaceDecl(Ptr<Cangjie::AST::Node> node, TypeManager* typeManager);
};
} // namespace Cangjie::CodeCheck

#endif // CANGJIECODECHECK_STRUCTURALRULEGITF01_H
