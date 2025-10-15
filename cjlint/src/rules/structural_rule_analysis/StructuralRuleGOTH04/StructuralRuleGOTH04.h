// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRGOTH04_H
#define CANGJIECODECHECK_STRUCTURALRGOTH04_H

#include "nlohmann/json.hpp"
#include "cangjie/AST/Match.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.OTH.04 不要使用String 存储敏感数据，敏感数据使用结束后应立即清 0
 */
class StructuralRuleGOTH04 : public StructuralRule {
public:
    explicit StructuralRuleGOTH04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGOTH04() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;
private:
    nlohmann::json jsonInfo;
    void FindVarDecl(Ptr<Cangjie::AST::Node> node);
    void AnalyzeVarDecl(const Cangjie::AST::VarDecl &varDecl);
    void VarDeclTypeAnalysis(Ptr<Cangjie::AST::Ty> ty, bool &flag);
    void TypeStructAnalysis(Ptr<Cangjie::AST::Ty> ty, bool &flag);
    void TypeEnumAnalysis(Ptr<Cangjie::AST::Ty> ty, bool &flag);
    void TypeTupleAnalysis(Ptr<Cangjie::AST::Ty> ty, bool &flag);
    bool SenInfoFilter(const std::string& key, const std::string& text);
    bool IsSensitiveDataVar(const std::string &text);

    void TypeClassAnalysis(Ptr<Cangjie::AST::Ty> ty, bool& flag);
    void IsIncludeStringType(Ptr<Cangjie::AST::Ty> ty, bool& flag);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURALRGOTH04_H