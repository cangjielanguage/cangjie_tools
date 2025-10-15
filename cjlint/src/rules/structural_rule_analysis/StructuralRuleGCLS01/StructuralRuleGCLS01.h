// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGCLS01_H
#define CANGJIECODECHECK_STRUCTURALRULEGCLS01_H

#include "cangjie/AST/Match.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGCLS01 : public StructuralRule {
public:
    explicit StructuralRuleGCLS01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGCLS01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
    void FindClassDecl(Ptr<Cangjie::AST::Node> node);
    void CheckClassDecl(const Cangjie::AST::ClassDecl &classDecl);
    void CheckSubClassFuncAccess(const Cangjie::AST::ClassDecl &classDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility,
        const std::string &parentClassName, std::set<Ptr<Cangjie::AST::Decl>> &visitedClassDecl);
    void CheckSubClassFuncAccessHelper(Ptr<Cangjie::AST::ClassDecl> classDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility,
        const std::string &parentClassName, std::set<Ptr<Cangjie::AST::Decl>> &visitedClassDecl);
    static void CheckParentClassFuncAccess(const Cangjie::AST::ClassDecl &classDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility);
    static bool CheckFuncCall(Ptr<Cangjie::AST::FuncDecl> funcDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility,
        const std::string &parentClassName, const std::string &subClassName, const std::string &access);
    static bool CheckFuncCallHelper(Ptr<Cangjie::AST::FuncDecl> funcDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility,
        const std::string &parentClassName, const std::string &subClassName, const std::string &access);
    static std::string CheckModifier(const std::set<Cangjie::AST::Modifier> &modifiers);
    static std::pair<std::string, std::vector<std::string>> GetFuncDecl(Ptr<Cangjie::AST::FuncDecl> funcDecl);
};
}
#endif // CANGJIECODECHECK_STRUCTURALRULEGCLS01_H
