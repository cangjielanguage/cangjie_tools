// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_OTH_03
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_OTH_03

#include <fstream>
#include <iostream>
#include "rules/structural_rule_analysis/RegexRule.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.OTH.03  禁止代码中包含公网地址
 */


class StructuralRuleGOTH03 : public RegexRule {
public:
    explicit StructuralRuleGOTH03(CodeCheckDiagnosticEngine *diagEngine) : RegexRule(diagEngine) {};
    ~StructuralRuleGOTH03() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    bool CheckIpAddress(const std::string ip);
    bool isIpHardcode(std::vector<int> ips);
    bool IsSpecialIpHardcode(std::vector<int> ips) const;
    void RecordErrorLocation(const std::vector<ResultInfo>::iterator iter, CodeCheckDiagKind kind);
};
} // namespace Cangjie::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_IP_HARDCODE
