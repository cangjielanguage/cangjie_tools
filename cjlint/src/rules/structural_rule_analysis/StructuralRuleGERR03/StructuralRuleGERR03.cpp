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
        VisitAction action = VisitAction::WALK_CHILDREN;
        match (*node)(
            [this](const MemberAccess &memberAccess) {
                if (inNoneValueExceptionTry == 0) {
                    CheckMemberAccessAttribute(memberAccess);
                }
            },
            [this, &action](const TryExpr &tryExpr) {
                if (!IsCatchingNoneValueException(tryExpr)) {
                    return;
                }
                ProcessTryExpr(tryExpr);
                action = VisitAction::SKIP_CHILDREN;
            });
        return action;
    });
    walker.Walk();
}

bool StructuralRuleGERR03::IsCatchingNoneValueException(const Cangjie::AST::TryExpr &tryExpr)
{
    for (auto &catchPattern : tryExpr.catchPatterns) {
        if (catchPattern == nullptr) {
            continue;
        }
        if (catchPattern->astKind == ASTKind::WILDCARD_PATTERN) {
            return true;
        }
        if (catchPattern->astKind != ASTKind::EXCEPT_TYPE_PATTERN) {
            continue;
        }
        auto etp = static_cast<const ExceptTypePattern *>(catchPattern.get().get());
        for (auto &type : etp->types) {
            if (type == nullptr) {
                continue;
            }
            auto refType = dynamic_cast<const RefType *>(type.get().get());
            if (refType != nullptr && refType->ref.identifier.Val() == "NoneValueException") {
                return true;
            }
        }
    }
    return false;
}

void StructuralRuleGERR03::ProcessTryExpr(const Cangjie::AST::TryExpr &tryExpr)
{
    inNoneValueExceptionTry++;
    if (tryExpr.tryBlock != nullptr) {
        FindMemberAccess(tryExpr.tryBlock.get());
    }
    inNoneValueExceptionTry--;
    for (auto &catchBlock : tryExpr.catchBlocks) {
        if (catchBlock != nullptr) {
            FindMemberAccess(catchBlock.get());
        }
    }
    if (tryExpr.finallyBlock != nullptr) {
        FindMemberAccess(tryExpr.finallyBlock.get());
    }
}

void StructuralRuleGERR03::CheckMemberAccessAttribute(const Cangjie::AST::MemberAccess &memberAccess)
{
    if (memberAccess.baseExpr == nullptr || memberAccess.baseExpr->GetTy() == nullptr) {
        return;
    }
    if (memberAccess.field == "getOrThrow" && memberAccess.baseExpr->GetTy()->name == "Option") {
        Diagnose(memberAccess.baseExpr->end, memberAccess.baseExpr->end + 1,
            CodeCheckDiagKind::G_ERR_03_avoid_option_getorthrow);
    }
}

void StructuralRuleGERR03::MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node)
{
    (void)ctx;
    FindMemberAccess(node);
}
}
