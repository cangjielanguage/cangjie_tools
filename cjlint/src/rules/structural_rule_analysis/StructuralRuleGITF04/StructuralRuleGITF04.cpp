// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGITF04.h"
namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGITF04::CheckFuncDeclParams(const Cangjie::AST::FuncDecl &funcDecl)
{
    if (funcDecl.funcBody == nullptr) {
        return;
    }
    if (funcDecl.ty) {
        Ptr<AST::FuncTy> funcTy = DynamicCast<AST::FuncTy*>(funcDecl.ty);
        if (funcTy && funcTy->retTy && funcTy->retTy->IsInterface()) {
            Diagnose(funcDecl.begin, funcDecl.end,
                CodeCheckDiagKind::G_ITF_04_avoid_directly_using_interfaces_as_types_02, funcDecl.identifier.Val());
        }
    }

    for (auto &paramList : funcDecl.funcBody->paramLists) {
        for (auto &param : paramList->params) {
            if (param->ty->IsInterface()) {
                Diagnose(param->begin, param->end,
                    CodeCheckDiagKind::G_ITF_04_avoid_directly_using_interfaces_as_types_01, param->identifier.Val());
            }
        }
    }
}

void StructuralRuleGITF04::FindFuncDecl(Cangjie::AST::Node *node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Node *node) -> VisitAction {
        match (*node)([this](const FuncDecl &funcDecl) { CheckFuncDeclParams(funcDecl); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGITF04::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindFuncDecl(node);
}
}