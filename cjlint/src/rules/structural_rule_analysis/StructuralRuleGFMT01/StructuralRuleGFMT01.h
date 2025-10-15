// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_01_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_01_H

#include <fstream>
#include <iostream>
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * 仓颉编程语言通用编程规范的0.1版本
 * G.FMT.01 源文件编码格式（包括注释）必须是UTF-8
 */
class StructuralRuleGFMT01 : public StructuralRule {
public:
    explicit StructuralRuleGFMT01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGFMT01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    std::string filepath;
    Cangjie::Position pos;
    void GetFileFromNode(Ptr<Cangjie::AST::Node> node);
    bool CheckUTF8File(const std::string &filePath);
    bool CheckUTF8Text(unsigned char *start, unsigned char *end) const;
    bool IsEmptyFile(const std::string &path) const;
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FMT_01_H