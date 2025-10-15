// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURAL_RULE_G_ENU_02_H
#define CANGJIECODECHECK_STRUCTURAL_RULE_G_ENU_02_H

#include <fstream>
#include <iostream>
#include "rules/structural_rule_analysis/StructuralRule.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGENU02 : public StructuralRule {
public:
    explicit StructuralRuleGENU02(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGENU02() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    struct EnumCtr {
        std::string identifier;
        std::vector<AST::Ty*> args;
        bool isCtr;
        EnumCtr(std::string identifier,
            std::vector<AST::Ty*> args, bool isCtr) : identifier(identifier), args(args), isCtr(isCtr){};
    };
    /** @brief Records information about all enum constructors. */
    std::vector<EnumCtr> enumCtrSet;
    /** @brief Map describing class inheritance introduced by extendDecl. */
    std::map<AST::Ty*, std::vector<AST::Type*>> inheritedClassMap;
    /** @brief Check whether there is an inheritance relationship between two Ty. */
    bool CheckTyEqualityHelper(Cangjie::AST::Ty* base, Cangjie::AST::Ty* derived);
    /** @brief If there is an inheritance relationship, the two types are considered to be the same. */
    bool IsEqual(Cangjie::AST::Ty* base, Cangjie::AST::Ty* derived);
    /** @brief Check and collect constructors of enum. */
    void FindEnumDeclHelper(Ptr<Cangjie::AST::Node> node);
    /** @brief Record the inheritance relationship implemented through Extend. */
    void FindExtendHelper(Ptr<Cangjie::AST::Node> node);
    /** @brief Check whether a function or constructor is duplicated. */
    void DuplicatedEnumCtrOrFuncHelper(const Cangjie::AST::FuncDecl& funcDecl);
    /** @brief Check whether enum constructor is overloaded. */
    void CheckEnumCtrOverload(const Cangjie::AST::EnumDecl& enumDecl);
    /** @brief Check whether a function is overloaded. */
    void CheckFuncOverload(const Cangjie::AST::FuncDecl& funcDecl);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURAL_RULE_G_ENU_02_H
