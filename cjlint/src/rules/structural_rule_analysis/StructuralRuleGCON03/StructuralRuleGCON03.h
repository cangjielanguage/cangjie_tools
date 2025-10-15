// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_03_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_03_H

#include "cangjie/AST/Match.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.CON.03 禁止使用非线程安全的函数来覆写线程安全的函数
 */
class StructuralRuleGCON03 : public StructuralRule {
public:
    enum class MutexState {
        NOT_MUTEX = 0,
        MUTEX_LOCK,
        MUTEX_UNLOCK
    };
    explicit StructuralRuleGCON03(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGCON03() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    using PositionPair = std::pair<Position, Position>;
    std::set<std::pair<std::pair<std::string, std::string>, PositionPair>> nonThreadSafeOverrideFuncSet;
    std::set<std::pair<std::string, std::string>> threadSafeBaseFuncSet;
    std::set<Cangjie::Position> safeFuncSet;
    std::set<Cangjie::Position> checkedFuncSet;
    void OverrideFuncFinder(Ptr<Cangjie::AST::Node> node);
    void BaseClassFinder(Ptr<Cangjie::AST::Node> node);
    void CheckBaseClassFuncSafe(std::vector<OwnedPtr<Cangjie::AST::Decl>> &decls, const std::string &className);
    bool CoverDeclToFuncDecl(Ptr<Cangjie::AST::Decl> decl);
    static bool IsOverrideModifier(const std::set<Cangjie::AST::Modifier> &modifiers);
    bool IsThreadSafe(std::vector<OwnedPtr<Cangjie::AST::Node>> &nodes);
    bool IsFuncSafe(Ptr<Cangjie::AST::CallExpr> callExpr);
    static bool IsAssignSafe(Ptr<Cangjie::AST::AssignExpr> assignExpr);
    static bool IsIncOrDecSafe(Ptr<Cangjie::AST::IncOrDecExpr> incOrDecExpr);
    static MutexState IsReentrantMutex(Ptr<Cangjie::AST::CallExpr> callExpr);
    void ResultCheck(
        const std::set<std::pair<std::pair<std::string, std::string>, PositionPair>> &unsafeThreadOverrideFuncSet,
        const std::set<std::pair<std::string, std::string>> &safeThreadBaseFuncSet);
};
}
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_03_H
