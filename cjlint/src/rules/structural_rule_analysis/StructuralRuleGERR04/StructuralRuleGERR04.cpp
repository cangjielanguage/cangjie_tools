// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGERR04.h"
#include "common/CommonFunc.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGERR04::CheckNode(Ptr<Cangjie::AST::Node> node, bool inChildScope)
{
    if (!node) {
        return;
    }

    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const JumpExpr& jumpExpr) {
                if (jumpExpr.isBreak) {
                    Diagnose(jumpExpr.begin, jumpExpr.end, CodeCheckDiagKind::G_ERR_04_avoid_break_finally);
                } else {
                    Diagnose(jumpExpr.begin, jumpExpr.end, CodeCheckDiagKind::G_ERR_04_avoid_continue_finally);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ReturnExpr& returnExpr) {
                Diagnose(returnExpr.begin, returnExpr.end, CodeCheckDiagKind::G_ERR_04_avoid_return_finally);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ThrowExpr& throwExpr) {
                Diagnose(throwExpr.begin, throwExpr.end, CodeCheckDiagKind::G_ERR_04_avoid_throw_finally);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const WhileExpr&) { return VisitAction::SKIP_CHILDREN; },
            [this](const ForInExpr&) { return VisitAction::SKIP_CHILDREN; },
            [this](const DoWhileExpr&) { return VisitAction::SKIP_CHILDREN; },
            [this](const FuncDecl&) { return VisitAction::SKIP_CHILDREN; },
            [this](const LambdaExpr&) { return VisitAction::SKIP_CHILDREN; },
            [this](const TryExpr&) { return VisitAction::SKIP_CHILDREN; }, []() { return VisitAction::WALK_CHILDREN; });
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleGERR04::FindFinallyBlock(Ptr<Cangjie::AST::Node> node)
{
    if (!node) {
        return;
    }

    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const TryExpr& tryExpr) {
            auto& finallyBlock = tryExpr.finallyBlock;
            if (!finallyBlock) {
                return;
            }
            for (auto& subNode : finallyBlock->body) {
                CheckNode(subNode);
            }
        });
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleGERR04::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindFinallyBlock(node);
}
} // namespace Cangjie::CodeCheck