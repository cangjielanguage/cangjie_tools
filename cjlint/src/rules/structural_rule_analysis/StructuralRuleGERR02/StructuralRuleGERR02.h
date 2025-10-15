// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGERR02_H
#define CANGJIECODECHECK_STRUCTURALRULEGERR02_H

#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/StructuralRule.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGERR02 : public StructuralRule {
public:
    explicit StructuralRuleGERR02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGERR02() override  = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    using PositionPair = std::pair<Cangjie::Position, Cangjie::Position>;
    std::vector<std::pair<PositionPair, std::string>> exceptionTypeDiag;
    std::vector<std::pair<PositionPair, std::string>> exceptionInfoDiag;

    void CheckResult(Ptr<Cangjie::AST::Node> node);
    std::string GetExceptionInfo(Ptr<Cangjie::AST::CallExpr> callExpr);
    std::string GetBinaryStr(Ptr<Cangjie::AST::BinaryExpr> binaryExpr);
    std::string GetDeclStr(Ptr<Cangjie::AST::Decl> decl);
    std::vector<std::string> GetSensitiveKeyWord(const std::string &input);
    bool IsSensitiveException(const std::string &input) const;
    void PrintDebugInfo() const;
    std::vector<std::pair<std::string, int>> debugInfo;
};
}

#endif
