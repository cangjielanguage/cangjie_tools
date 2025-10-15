// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_STRUCTURALRULEGEXP05_H
#define CANGJIECODECHECK_STRUCTURALRULEGEXP05_H

#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
class StructuralRuleGEXP05 : public StructuralRule {
public:
    explicit StructuralRuleGEXP05(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGEXP05() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    std::unordered_map<Cangjie::TokenKind, std::unordered_set<Cangjie::TokenKind>> ConfusOperMap = {
        {TokenKind::ADD,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::SUB,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::MUL,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::DIV,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::MOD,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::EXP,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::LT, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::LT, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::LE, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::GT, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::GE, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::EQUAL,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::NOTEQ,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::LSHIFT,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}},
        {TokenKind::RSHIFT,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}},
        {TokenKind::BITAND,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}},
        {TokenKind::BITOR,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}},
        {TokenKind::BITXOR,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}}};
    void FindParenExpr(Cangjie::AST::Node* node);
    void CheckParenExpr(const Cangjie::AST::ParenExpr& parenExpr);
    void CheckBinaryExpr(const Cangjie::AST::BinaryExpr& binaryExpr);
    void CheckSubBinaryExpr(AST::Expr* subExpr, const Cangjie::TokenKind& op);
};
} // namespace Cangjie::CodeCheck
#endif // CANGJIECODECHECK_STRUCTURALRULEGEXP05_H
