// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGCON02_H
#define CANGJIECODECHECK_STRUCTURALRULEGCON02_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
using Gcon02Lock = std::map<std::string, std::vector<int>>;

const int TUPLE_PARAM_1ST = 0;
const int TUPLE_PARAM_2ND = 1;
const int TUPLE_PARAM_3RD = 2;
const int TUPLE_PARAM_4TH = 3;

class StructuralRuleGCON02 : public StructuralRule {
public:
    explicit StructuralRuleGCON02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGCON02() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    std::unordered_map<std::string, std::string> varMap;
    std::unordered_map<std::string, std::tuple<Cangjie::Position, Cangjie::Position, bool, std::string>> tryLockMap;
    std::set<std::string> tryBlockKey;

    void CheckResult(Ptr<Cangjie::AST::Node> node);
    void CheckTryExpr(const Cangjie::AST::TryExpr &tryExpr);
    void PrintDiagInfo();
    void TryLockCheck(const Cangjie::AST::MemberAccess &memberAccess);
    void FinallyUnLockCheck(const Cangjie::AST::MemberAccess &memberAccess);
    std::string GetMemberAccessField(const Cangjie::AST::MemberAccess &memberAccess);
    void GetLockInfo(const Cangjie::AST::VarDecl &varDecl);
    std::string GetLockKey(const Cangjie::AST::VarDecl &varDecl);
    std::string GetMemberAccessKey(const Cangjie::AST::MemberAccess &memberAccess);
};
}
#endif
