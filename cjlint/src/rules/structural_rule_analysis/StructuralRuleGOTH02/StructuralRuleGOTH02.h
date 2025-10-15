// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_OTH_02
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_OTH_02

#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/RegexRule.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.OTH.02 禁止将敏感信息硬编码在程序中
 */
class StructuralRuleGOTH02 : public RegexRule {
public:
    explicit StructuralRuleGOTH02(CodeCheckDiagnosticEngine* diagEngine) : RegexRule(diagEngine){};
    ~StructuralRuleGOTH02() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    void FindSenInfoInAST(Ptr<Cangjie::AST::Node> node);
    bool IsWeakPasscode(const std::string& text);
    bool IsSenInfoName(const std::string& text);
    void MatchPatternInAST(const Cangjie::AST::VarDecl& varDecl);
    void MatchPatternInAST(const Cangjie::AST::VarWithPatternDecl& varWithPatternDecl);
    void DealWithMatchResultfromRegex(Ptr<Cangjie::AST::Node> node);
    bool SenInfoFilter(const std::string& key, const std::string& text);
};
} // namespace Cangjie::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_SYNTAX_RULE_G_OTH_02