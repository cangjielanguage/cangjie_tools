// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGITF02.h"
#include "common/CommonFunc.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace AST;
using namespace Meta;

void StructuralRuleGITF02::CheckExtendMemberDeclsHelper(
    Ptr<Cangjie::AST::InterfaceDecl> interface, Ptr<Cangjie::AST::Decl> member, bool& label)
{
    for (auto& memberDecl : interface->GetMemberDecls()) {
        if (memberDecl->identifier == member->identifier && memberDecl->ty == member->ty &&
            !CommonFunc::IsStdDerivedMacro(diagEngine, member->begin)) {
            Diagnose(member->begin, member->end,
                CodeCheckDiagKind::G_ITF_02_prefer_implement_interfaces_at_type_definition, member->identifier.Val());
            label = true;
            break;
        }
    }
}

void StructuralRuleGITF02::CheckExtendMemberDecls(const Cangjie::AST::ExtendDecl& extendDecl)
{
    if (extendDecl.GetSuperInterfaceTys().empty()) {
        return;
    }
    for (auto& member : extendDecl.members) {
        bool label = false;
        for (auto& interfaceTy : extendDecl.GetSuperInterfaceTys()) {
            auto interface = RawStaticCast<AST::InterfaceDecl*>(AST::Ty::GetDeclOfTy(interfaceTy));
            if (interface->TestAttr(Attribute::IMPORTED)) {
                continue;
            }
            CheckExtendMemberDeclsHelper(interface, member.get(), label);
            if (label) {
                break;
            }
        }
    }
}

void StructuralRuleGITF02::FindExtendDecl(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const ExtendDecl& extendDecl) {
                CheckExtendMemberDecls(extendDecl);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGITF02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindExtendDecl(node);
}
} // namespace Cangjie::CodeCheck
