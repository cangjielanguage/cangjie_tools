// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGSEC01_H
#define CANGJIECODECHECK_STRUCTURALRULEGSEC01_H

#include <fstream>
#include <iostream>
#include <regex>
#include "cangjie/AST/Match.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"
#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.SEC.01 进行安全检查的方法禁止声明为open
 */

class StructuralRuleGSEC01 : public StructuralRule {
public:
    explicit StructuralRuleGSEC01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGSEC01() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    void FindCheckingFunction(Ptr<Cangjie::AST::Node> node);
    void ClassDeclHandler(const Cangjie::AST::ClassDecl& classDecl);
    void RecordLocation(Ptr<AST::FuncDecl> funcDecl);
    void InterfaceDeclHandler(const Cangjie::AST::InterfaceDecl& interfaceDecl);
    bool IsExtendClass(const Cangjie::AST::ClassDecl& classDecl) const;
    void ClassDeclHandlerDetail(const OwnedPtr<Cangjie::AST::Decl>& classBody, const bool isExtend);
};
} // namespace Cangjie::CodeCheck

#endif // CANGJIECODECHECK_STRUCTURALRULEGSEC01_H
