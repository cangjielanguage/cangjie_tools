// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGNAM03_H
#define CANGJIECODECHECK_STRUCTURALRULEGNAM03_H

#include <fstream>
#include <iostream>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.NAM.03 接口，类，Struct、枚举类型和枚举成员构造，类型别名，采用大驼峰命名
 */
class StructuralRuleGNAM03 : public StructuralRule {
public:
    explicit StructuralRuleGNAM03(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGNAM03() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FindAssignTypes(Ptr<Cangjie::AST::Node> node);
    void CheckExceptionRule(const Cangjie::AST::ClassDecl& classDecl);
    template <typename T> auto CheckNameRule(T& decl);
    bool IsExceptionSubclass(const Cangjie::AST::ClassDecl& decl, std::set<Ptr<AST::ClassDecl>> declSet = {});
    void CheckQuoteExprToken(std::vector<Token>& tokens);
    void CheckQuoteExpr(const Cangjie::AST::QuoteExpr& quoteExpr);
    void PrintDiagnoseInfo(bool condition, Position start, Position end, const std::string& value);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURALRULEGNAM03_H
