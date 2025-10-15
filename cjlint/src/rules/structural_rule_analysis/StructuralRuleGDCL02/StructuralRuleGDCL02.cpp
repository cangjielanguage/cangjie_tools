// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGDCL02.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

static bool IsPublic(std::set<Cangjie::AST::Modifier>& modifiers)
{
    auto item = std::find_if(
        modifiers.begin(), modifiers.end(), [](auto& modifier) { return modifier.modifier == TokenKind::PUBLIC; });
    return item != modifiers.end();
}

static bool IsOperator(std::set<Cangjie::AST::Modifier>& modifiers)
{
    auto item = std::find_if(
        modifiers.begin(), modifiers.end(), [](auto& modifier) { return modifier.modifier == TokenKind::OPERATOR; });
    return item != modifiers.end();
}

void StructuralRuleGDCL02::CheckReturnValType(Ptr<Node> node)
{
    if (!node) {
        return;
    }
    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::FUNC_DECL) {
            auto funcDecl = StaticCast<FuncDecl*>(node);
            if (funcDecl->IsFinalizer() || funcDecl->TestAttr(AST::Attribute::CONSTRUCTOR)) {
                return VisitAction::WALK_CHILDREN;
            }
            if (IsOperator(funcDecl->modifiers)) {
                return VisitAction::WALK_CHILDREN;
            }
            if (IsPublic(funcDecl->modifiers) && funcDecl->funcBody && !funcDecl->funcBody->retType) {
                Diagnose(funcDecl->identifier.Begin(), funcDecl->identifier.End(),
                    CodeCheckDiagKind::G_DCL_02_public_function_type, funcDecl->identifier.GetRawText());
            }
            return VisitAction::WALK_CHILDREN;
        }
        if (node->astKind == ASTKind::VAR_DECL) {
            auto varDecl = StaticCast<VarDecl*>(node);
            if (IsPublic(varDecl->modifiers) && !varDecl->type) {
                Diagnose(varDecl->identifier.Begin(), varDecl->identifier.End(),
                    CodeCheckDiagKind::G_DCL_02_public_variable_type, varDecl->identifier.GetRawText());
            }
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleGDCL02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckReturnValType(node);
}
} // namespace Cangjie::CodeCheck