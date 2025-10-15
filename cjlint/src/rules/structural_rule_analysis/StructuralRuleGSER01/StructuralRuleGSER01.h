// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_01_H
#define CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_01_H

#include "cangjie/Basic/Match.h"
#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/StructuralRuleGSER.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGSER01 : public StructuralRuleGSER {
public:
    explicit StructuralRuleGSER01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRuleGSER(diagEngine){};
    ~StructuralRuleGSER01() override = default;

protected:
    void MatchPattern(ASTContext&, Ptr<Cangjie::AST::Node> node) override;

private:
    nlohmann::json sensitiveKeys;
    const std::string* GetStringExprVal(Ptr<Cangjie::AST::Expr> expr);
    void CheckSensitiveInfo(Ptr<Cangjie::AST::Expr> expr, CodeCheckDiagKind kind);
    void CheckSensitiveInfo(
        const std::string& str, CodeCheckDiagKind kind, const Cangjie::Position& start, const Cangjie::Position& end);
    void SerializeHandler(const Cangjie::AST::FuncDecl& funcDecl);
    template <typename T> void JudgeSerialize(const T& decl);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_01_H
