// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGERR03.h"
#include "common/CommonFunc.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGERR03::FindMemberAccess(Ptr<Cangjie::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const MemberAccess &memberAccess) { CheckMemberAccessAttribute(memberAccess); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGERR03::CheckMemberAccessAttribute(const Cangjie::AST::MemberAccess &memberAccess)
{
    if (memberAccess.baseExpr == nullptr || memberAccess.baseExpr->ty == nullptr) {
        return;
    }
    if (memberAccess.field == "getOrThrow" && memberAccess.baseExpr->ty->name == "Option") {
        Diagnose(memberAccess.baseExpr->end, memberAccess.baseExpr->end + 1,
            CodeCheckDiagKind::G_ERR_03_avoid_option_getorthrow);
    }
}

void StructuralRuleGERR03::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindMemberAccess(node);
}
}