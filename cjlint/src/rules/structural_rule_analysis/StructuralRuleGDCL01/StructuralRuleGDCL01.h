// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_01_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_01_H
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.DCL.01 避免遮盖（shadow）
 */
class StructuralRuleGDCL01 : public StructuralRule {
public:
    explicit StructuralRuleGDCL01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGDCL01() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FindShadowNode(Ptr<Cangjie::AST::Node>& node);
    template <typename T>
    void TraversingNode(std::vector<OwnedPtr<T>>& node,
        std::map<std::string, std::map<std::string, std::stack<AST::Decl*>>>& container,
        std::map<std::string, std::map<std::string, std::vector<AST::Decl*>>>& tmpContainer, bool isStaticFunc = false);
    void VarDeclProcessor(Cangjie::AST::VarDecl& varDecl, std::map<std::string, std::stack<AST::Decl*>>& varContainer,
        std::map<std::string, std::vector<AST::Decl*>>& varTmpContainer, bool isStaticFunc = false);
    void FuncDeclProcessor(AST::FuncDecl& funcDecl, std::map<std::string, std::stack<AST::Decl*>>& funcContainer);
    static void PostFuncDeclProcessor(
        Cangjie::AST::FuncDecl& funcDecl, std::map<std::string, std::stack<AST::Decl*>>& funcContainer);
    static void PostVarDeclProcessor(AST::VarDecl& varDecl, std::map<std::string, std::stack<AST::Decl*>>& varContainer,
        std::set<std::string>& varSet);
    template <typename T>
    static void PostBlockAndClassBodyProcessor(std::vector<OwnedPtr<T>>& nodes,
        std::map<std::string, std::map<std::string, std::stack<AST::Decl*>>>& container,
        std::map<std::string, std::map<std::string, std::vector<AST::Decl*>>>& tmpContainer);
    void FindGenericShadowNode(Ptr<Cangjie::AST::Node>& node);
    template <typename T>
    void MemberDeclProcessor(
        const std::vector<OwnedPtr<T>>& members, std::map<std::string, AST::Decl*>& genericContainer);
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_01_H