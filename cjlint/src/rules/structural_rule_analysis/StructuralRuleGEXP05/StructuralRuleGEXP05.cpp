// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGEXP05.h"

using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;
using namespace CodeCheck;

static std::unordered_map<Cangjie::TokenKind, std::string> unaryOpToStringMap = {
    {TokenKind::NOT, "!"}, {TokenKind::SUB, "-"}};

void StructuralRuleGEXP05::CheckParenExpr(const Cangjie::AST::ParenExpr& parenExpr)
{
    // Check if the parentheses are redundant.
    // For example, '(!a)', '(-a)'.
    if (parenExpr.expr && parenExpr.expr->astKind == ASTKind::UNARY_EXPR) {
        auto op = RawStaticCast<AST::UnaryExpr*>(parenExpr.expr.get())->op;
        Diagnose(parenExpr.begin, parenExpr.end,
            CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_01,
            unaryOpToStringMap.count(op) > 0 ? unaryOpToStringMap[op] : "");
    }
    // For example, '((a))'
    if (parenExpr.expr && parenExpr.expr->astKind == ASTKind::PAREN_EXPR) {
        Diagnose(parenExpr.begin, parenExpr.end,
            CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_03, "");
    }
}

void StructuralRuleGEXP05::CheckSubBinaryExpr(AST::Expr* subExpr, const Cangjie::TokenKind& op)
{
    // Check if the parentheses are redundant
    // for example, '(a + b) + c'
    if (subExpr->astKind == ASTKind::PAREN_EXPR) {
        auto parenExpr = RawStaticCast<AST::ParenExpr*>(subExpr);
        if (parenExpr->expr && parenExpr->expr->astKind == ASTKind::BINARY_EXPR) {
            auto subBinaryExpr = RawStaticCast<AST::BinaryExpr*>(parenExpr->expr.get());
            if (subBinaryExpr->op == op) {
                Diagnose(parenExpr->begin, parenExpr->end,
                    CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_02, "");
            }
        }
        return;
    }
    // Check if parentheses are needed
    // For example, 'a << b < c'
    if (subExpr->astKind == ASTKind::BINARY_EXPR) {
        auto subBinaryExpr = RawStaticCast<AST::BinaryExpr*>(subExpr);
        if (ConfusOperMap.count(op) > 0 && ConfusOperMap[op].count(subBinaryExpr->op)) {
            Diagnose(subBinaryExpr->begin, subBinaryExpr->end,
                CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_04, "");
        }
    }
}

void StructuralRuleGEXP05::CheckBinaryExpr(const Cangjie::AST::BinaryExpr& binaryExpr)
{
    if (binaryExpr.leftExpr) {
        CheckSubBinaryExpr(binaryExpr.leftExpr.get(), binaryExpr.op);
    }
    if (binaryExpr.rightExpr) {
        CheckSubBinaryExpr(binaryExpr.rightExpr.get(), binaryExpr.op);
    }
}

void StructuralRuleGEXP05::FindParenExpr(Node* node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Node* node) -> VisitAction {
        return match(*node)(
            [this](const ParenExpr& parenExpr) {
                CheckParenExpr(parenExpr);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const BinaryExpr& binaryExpr) {
                CheckBinaryExpr(binaryExpr);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const TupleLit& tupleLit) {
                for (auto& child : tupleLit.children) {
                    if (child->astKind == ASTKind::PAREN_EXPR) {
                        Diagnose(child->begin, child->end,
                            CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_03, "");
                    }
                }
                return VisitAction::WALK_CHILDREN;
            },
            [this](const ArrayLit& arrayLit) {
                for (auto& child : arrayLit.children) {
                    if (child->astKind == ASTKind::PAREN_EXPR) {
                        Diagnose(child->begin, child->end,
                            CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_03, "");
                    }
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    
    walker.Walk();
}

void StructuralRuleGEXP05::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindParenExpr(node);
}
