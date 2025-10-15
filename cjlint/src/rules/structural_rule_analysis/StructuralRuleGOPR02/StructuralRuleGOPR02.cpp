// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGOPR02.h"

using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;
using namespace CodeCheck;

/**
 * Avoid defining `()` operator overloading functions in enumeration types.
 * wrong eg:
 * enum E {
 *     Y | X | X(Int64)
 *     operator func ()(a: Int64) {
 *         a
 *     }
 * }
 */
void StructuralRuleGOPR02::CheckParenthesesOpertorInEnum(EnumDecl& enumDecl)
{
    for (auto& decl : enumDecl.GetMemberDecls()) {
        if (decl->astKind == ASTKind::FUNC_DECL) {
            if (decl->TestAttr(Attribute::OPERATOR) && decl->identifier == "()") {
                Diagnose(decl->begin, decl->end,
                    CodeCheckDiagKind::G_OPR_02_enum_parentheses_overload_information, decl->identifier.Val());
            }
        }
    }
}

void StructuralRuleGOPR02::CheckParenthesesOpertorInExtend(ExtendDecl& extendDecl)
{
    if (!extendDecl.ty) {
        return;
    }
    if (extendDecl.ty->kind != TypeKind::TYPE_ENUM) {
        return;
    }
    for (auto& decl : extendDecl.GetMemberDecls()) {
        if (decl->astKind == ASTKind::FUNC_DECL) {
            if (decl->TestAttr(Attribute::OPERATOR) && decl->identifier == "()") {
                Diagnose(decl->begin, decl->end,
                    CodeCheckDiagKind::G_OPR_02_enum_parentheses_overload_information, decl->identifier.Val());
            }
        }
    }
}

void StructuralRuleGOPR02::FindEnum(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](EnumDecl& enumDecl) {
                CheckParenthesesOpertorInEnum(enumDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](ExtendDecl& extendDecl) {
                CheckParenthesesOpertorInExtend(extendDecl);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGOPR02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindEnum(node);
}
