// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGFMT15.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGFMT15::CheckLitConstExpr(const LitConstExpr& litConstExpr)
{
    if (!litConstExpr.ty || !litConstExpr.ty->IsFloating()) {
        return;
    }
    auto lst = FileUtil::SplitStr(litConstExpr.rawString, '.');
    if (lst.size() == 1 || (lst.size() > 1 && lst[0] == "-")) {
        Diagnose(litConstExpr.begin, litConstExpr.end, CodeCheckDiagKind::G_FMT_15_leading_zero_before_decimal,
            litConstExpr.rawString);
    }
}

void StructuralRuleGFMT15::FindLitConstExpr(Ptr<Cangjie::AST::Node> node)
{
    if (!node) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const LitConstExpr& litConstExpr) { CheckLitConstExpr(litConstExpr); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGFMT15::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindLitConstExpr(node);
}
} // namespace Cangjie::CodeCheck
