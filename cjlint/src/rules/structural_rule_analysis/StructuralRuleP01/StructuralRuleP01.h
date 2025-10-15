// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEP01_H
#define CANGJIECODECHECK_STRUCTURALRULEP01_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
class StructuralRuleP01 : public StructuralRule {
public:
    explicit StructuralRuleP01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleP01() override = default;
    void DoAnalysis(CJLintCompilerInstance *instance) override;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    using PositionPair = std::pair<Position, Position>;
    using P01DiagFmt = std::tuple<PositionPair, CodeCheckDiagKind, int, std::string>;
    using P01BlockSeq = std::vector<std::pair<std::string, int>>;
    using P01FuncBlocks = std::vector<std::pair<std::pair<PositionPair, std::string>, P01BlockSeq>>;
    std::unordered_map<std::string, P01FuncBlocks> funcLockSeq;

    void GetLockSeq(Ptr<Cangjie::AST::Node> node);
    void FuncHandler(Cangjie::AST::FuncBody &funcBody);
    void LambdaExprHandler(const Cangjie::AST::LambdaExpr &lambdaExpr);
    void SynchronizedExprHandler(Cangjie::AST::SynchronizedExpr &expr, const std::string funcName);
    void CompareTwoFuncBlocks(P01FuncBlocks &blockI, P01FuncBlocks &blockJ, std::vector<P01DiagFmt> &diagInfos);
    void CheckResult();
    void PrintDebugInfo() const;
};
}

#endif
