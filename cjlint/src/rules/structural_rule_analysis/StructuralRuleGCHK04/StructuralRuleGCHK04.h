// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_04_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_04_H

#include "nlohmann/json.hpp"
#include "cangjie/AST/Match.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"
#include "common/TaintData.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.CHK.04 禁止直接使用不可信数据构造正则表达式
 */
class StructuralRuleGCHK04 : public StructuralRule {
public:
    explicit StructuralRuleGCHK04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGCHK04() override = default;

protected:
    void MatchPattern(ASTContext&, Ptr<Cangjie::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    void RegexFinder(Ptr<Cangjie::AST::Node> node);
    void GetLitConstExpr(const Cangjie::AST::CallExpr &callExpr);
    void RegexChecker(const OwnedPtr<AST::FuncArg> &arg);
    void GetLitConstExprHelperRefExpr(Ptr<Cangjie::AST::Expr> expr, const Position start, const Position end,
        std::string refName, std::string &str);
    void MemberAccessTargetCheck(Ptr<Cangjie::AST::MemberAccess> memberAccess, const Position start, const Position end,
        const std::string refName, std::string &tmpStr);
    std::string GetLitConstExprHelper(Ptr<Cangjie::AST::Expr> expr, const Position start, const Position end,
        const std::string &refName = "");
    std::set<std::string, Cangjie::Position> regexSet;
};
} // namespace Cangjie::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_04_H
