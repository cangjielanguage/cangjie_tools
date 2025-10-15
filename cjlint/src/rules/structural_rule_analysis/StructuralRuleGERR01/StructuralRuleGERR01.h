// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGERR01_H
#define CANGJIECODECHECK_STRUCTURALRULEGERR01_H

#include <vector>
#include <string>
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGERR01 : public StructuralRule {
public:
    explicit StructuralRuleGERR01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGERR01() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<AST::Node> node) override;
    void CheckFuncDecl(Ptr<AST::Node>& node);
    void CheckFuncDeclHelper(const Ptr<AST::FuncDecl>& funcDecl, const std::vector<AST::CommentGroup>& vector);
    void AnalyzeFunctionDecl(Ptr<AST::Node> node, const std::vector<AST::CommentGroup>& comments);
    void AnalyzeCatchBlock(std::vector<OwnedPtr<AST::Block>>& blocks);
    static bool CommentsIncludeExceptionInfo(
        const std::vector<AST::CommentGroup>& comments, const std::string& exception);
    void AnalyzeThrowExpr(const Ptr<AST::ThrowExpr>& throwExpr, const std::vector<AST::CommentGroup>& comments);
};
} // namespace Cangjie::CodeCheck

#endif
