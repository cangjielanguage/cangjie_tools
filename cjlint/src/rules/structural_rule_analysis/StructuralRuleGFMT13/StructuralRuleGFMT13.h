// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_13_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_13_H
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * 仓颉编程语言通用编程规范的0.1版本
 * G.FMT.13 文件头注释应该包含版权许可
 */
class StructuralRuleGFMT13 : public StructuralRule {
public:
    explicit StructuralRuleGFMT13(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGFMT13() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    void FindComments(Ptr<Cangjie::AST::Node>& node);
    void GetTopLevelComments(Ptr<Cangjie::AST::File> pFile);
    void AnalyzeComments(std::vector<AST::CommentGroup>& leadingComments, Cangjie::Position& pos);
    static bool ContainsCopyright(const std::string& str);
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_13_H