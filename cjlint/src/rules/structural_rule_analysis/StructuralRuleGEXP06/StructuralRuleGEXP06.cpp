// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGEXP06.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGEXP06::FindBinaryExpr(Ptr<Cangjie::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const BinaryExpr &binaryExpr) { CheckBinaryExpr(binaryExpr); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGEXP06::CheckBinaryExpr(const Cangjie::AST::BinaryExpr &binaryExpr)
{
    if (binaryExpr.op != TokenKind::EQUAL && binaryExpr.op != TokenKind::NOTEQ) {
        return;
    }
    if (binaryExpr.leftExpr == nullptr || binaryExpr.rightExpr == nullptr) {
        return;
    }
    if (binaryExpr.leftExpr->ty == nullptr || binaryExpr.rightExpr->ty == nullptr) {
        return;
    }
    if (binaryExpr.leftExpr->ty->IsBoolean() && binaryExpr.rightExpr->ty->IsBoolean()) {
        if (binaryExpr.leftExpr->astKind == AST::ASTKind::LIT_CONST_EXPR ||
            binaryExpr.rightExpr->astKind == AST::ASTKind::LIT_CONST_EXPR) {
            // 2 is the length of a binary operator.
            Diagnose(binaryExpr.operatorPos, binaryExpr.operatorPos + 2,
                CodeCheckDiagKind::G_EXP_06_avoid_redundant_op_in_bool_type_comparisons);
        }
    }
}

void StructuralRuleGEXP06::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindBinaryExpr(node);
}
}