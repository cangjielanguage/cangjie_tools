// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGTYP03.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGTYP03::AnalyzeBinaryExpr(const Ptr<Expr> expr1, const Ptr<Expr>& expr2)
{
    if (expr1->astKind == ASTKind::MEMBER_ACCESS) {
        auto ma = As<ASTKind::MEMBER_ACCESS>(expr1);
        if (!ma) {
            return;
        }
        if (ma->field == "NaN") {
            Diagnose(expr2->begin, expr2->end, CodeCheckDiagKind::G_TYP_03_use_isNaN_method_float);
        }
    }
}

void StructuralRuleGTYP03::CheckBinaryExpr(const BinaryExpr& binaryExpr)
{
    if (binaryExpr.op != TokenKind::EQUAL && binaryExpr.op != TokenKind::NOTEQ) {
        return;
    }
    if (!binaryExpr.leftExpr || !binaryExpr.rightExpr) {
        return;
    }
    if (!binaryExpr.leftExpr->ty->IsFloating() && !binaryExpr.rightExpr->ty->IsFloating()) {
        return;
    }
    AnalyzeBinaryExpr(binaryExpr.leftExpr, binaryExpr.rightExpr);
    AnalyzeBinaryExpr(binaryExpr.rightExpr, binaryExpr.leftExpr);
}

void StructuralRuleGTYP03::FloatBinaryFinder(Ptr<Cangjie::AST::Node> node)
{
    if (!node) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const BinaryExpr& binaryExpr) {
                CheckBinaryExpr(binaryExpr);
                return VisitAction::WALK_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGTYP03::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FloatBinaryFinder(node);
}
} // namespace Cangjie::CodeCheck
