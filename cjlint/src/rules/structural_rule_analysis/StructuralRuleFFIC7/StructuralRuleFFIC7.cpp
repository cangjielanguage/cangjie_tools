// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleFFIC7.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace AST;
using namespace Meta;

static Ptr<Ty> GetType(Ptr<Ty> ty)
{
    if (!ty || ty->typeArgs.empty()) {
        return nullptr;
    }
    return ty->typeArgs[0];
}

void StructuralRuleFFIC7::CheckPointerExpr(Ptr<AST::Node> node)
{
    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind != ASTKind::POINTER_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        auto pointerExpr = StaticAs<ASTKind::POINTER_EXPR>(node);
        auto toTy = GetType(pointerExpr->ty);
        if (!pointerExpr->arg || !pointerExpr->arg->ty) {
            return VisitAction::WALK_CHILDREN;
        }
        auto fromTy = GetType(pointerExpr->arg->ty);
        if (!toTy || !fromTy) {
            return VisitAction::WALK_CHILDREN;
        }
        if (toTy->kind != fromTy->kind) {
            Diagnose(pointerExpr->begin, pointerExpr->end, CodeCheckDiagKind::FFI_C_7_avoid_truncation_error, "");
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleFFIC7::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckPointerExpr(node);
}
} // namespace Cangjie::CodeCheck
