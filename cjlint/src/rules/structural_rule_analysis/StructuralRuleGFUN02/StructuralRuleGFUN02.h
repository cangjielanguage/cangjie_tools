// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_02_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_02_H
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * 仓颉编程语言通用编程规范的0.1版本
 * G.FUN.02 禁止函数有未被使用的参数
 */
class StructuralRuleGFUN02 : public StructuralRule {
public:
    explicit StructuralRuleGFUN02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGFUN02() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    using PositionPair = std::pair<Position, Position>;
    void FuncParamFinder(Ptr<Cangjie::AST::Node> node);
    void FuncParamFinderHelper(Ptr<Cangjie::AST::Node> node, std::map<std::string, PositionPair> &paramMap);
    static void EraseItemOfMap(const std::string& id, std::map<std::string, PositionPair>& paramMap);
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_02_H