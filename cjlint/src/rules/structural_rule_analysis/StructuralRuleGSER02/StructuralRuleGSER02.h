// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_02_H
#define CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_02_H

#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRuleGSER.h"

namespace Cangjie::CodeCheck {
/**
 * G.SER.02 防止反序列化被利用来绕过构造方法中的安全操作
 */

struct VarStruct {
    int directlyAssigned = 0;
    Ptr<const Cangjie::AST::VarDecl> varDecl;
};

class StructuralRuleGSER02 : public StructuralRuleGSER {
public:
    explicit StructuralRuleGSER02(CodeCheckDiagnosticEngine* diagEngine) : StructuralRuleGSER(diagEngine){};
    ~StructuralRuleGSER02() override = default;

protected:
    void MatchPattern(ASTContext&, Ptr<Cangjie::AST::Node> node) override;

private:
    std::map<std::string, VarStruct> varSet;
    using PositionPair = std::pair<Position, Position>;
    std::map<PositionPair, std::string> initDeserialVar;
    void JudgeSer(Ptr<Cangjie::AST::Node> node);
    void RecordLocation(const Cangjie::AST::RefExpr& ref);
    void DeserializeHandler(const Cangjie::AST::FuncDecl& funcDecl);
    void ConstructorHandler(const Cangjie::AST::FuncDecl& funcDecl);
    void CheckInit();
    template <typename T> inline void DeclHandler(const T& decl);
    template <typename T> inline auto SerJudgeHandler(const T& decl);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURAL_RULE_G_SER_02_H
