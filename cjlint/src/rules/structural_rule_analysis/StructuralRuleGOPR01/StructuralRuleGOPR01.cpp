// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGOPR01.h"

using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;
using namespace CodeCheck;

static const std::unordered_set<Cangjie::TokenKind> operatorSet = {TokenKind::ADD, TokenKind::SUB, TokenKind::MUT,
    TokenKind::DIV, TokenKind::LT, TokenKind::GT, TokenKind::LE, TokenKind::GE, TokenKind::NOTEQ, TokenKind::EQUAL};

void StructuralRuleGOPR01::CheckExtendDecl(Cangjie::AST::ExtendDecl& extendDecl)
{
    if (!extendDecl.ty->IsPrimitive()) {
        return;
    }
    for (auto& member : extendDecl.members) {
        if (member->astKind == ASTKind::FUNC_DECL) {
            auto funcDecl = static_cast<AST::FuncDecl*>(member.get().get());
            if (funcDecl->TestAttr(Attribute::OPERATOR)) {
                Diagnose(funcDecl->begin, funcDecl->end,
                    CodeCheckDiagKind::G_OPR_01_avoid_operator_overloading_that_violates_conventions_01,
                    funcDecl->identifier.Val());
            }
        }
    }
}

void StructuralRuleGOPR01::CheckFuncDecl(Cangjie::AST::FuncDecl& funcDecl)
{
    if (funcDecl.TestAttr(Attribute::OPERATOR)) {
        needDiag = true;
        if (operatorSet.count(funcDecl.op) > 0) {
            Walker walker(&funcDecl, [&funcDecl, this](Ptr<Node> node) -> VisitAction {
                return match(*node)(
                    [&funcDecl, this](BinaryExpr& binaryExpr) {
                        if (binaryExpr.op == funcDecl.op) {
                            needDiag = false;
                        }
                        return VisitAction::WALK_CHILDREN;
                    },
                    [this]() { return VisitAction::WALK_CHILDREN; });
            });
            walker.Walk();
        }
        if (needDiag) {
            Diagnose(funcDecl.begin, funcDecl.end,
                CodeCheckDiagKind::G_OPR_01_avoid_operator_overloading_that_violates_conventions_02,
                funcDecl.identifier.Val());
        }
    }
}
void StructuralRuleGOPR01::FindExtendDecl(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](ExtendDecl& extendDecl) {
                CheckExtendDecl(extendDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](FuncDecl& funcDecl) {
                CheckFuncDecl(funcDecl);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGOPR01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindExtendDecl(node);
}
