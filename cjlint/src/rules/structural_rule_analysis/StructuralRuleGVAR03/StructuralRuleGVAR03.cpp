// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGVAR03.h"

using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;
using namespace CodeCheck;

/**
 * Avoid using global variables
 * Print warning while declaring or referring global variables
 */

void StructuralRuleGVAR03::CheckVarDecl(const VarDecl &varDecl)
{
    if (varDecl.TestAttr(Attribute::GLOBAL)) {
        Diagnose(varDecl.begin, varDecl.end, CodeCheckDiagKind::G_VAR_03_global_variable_declaration_information,
            varDecl.identifier.Val());
    }
}

void StructuralRuleGVAR03::FindGlobalVar(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl &varDecl) {
                CheckVarDecl(varDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGVAR03::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindGlobalVar(node);
}
