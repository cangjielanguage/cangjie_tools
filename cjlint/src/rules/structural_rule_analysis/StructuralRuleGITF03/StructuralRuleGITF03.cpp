// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGITF03.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGITF03::CheckParent(const std::string &childName, const InterfaceTy *ty,
    const InheritableDecl &inheritableDecl, std::set<Ptr<InterfaceTy>> tySet)
{
    for (auto &type : inheritableDecl.GetSuperInterfaceTys()) {
        if (type->name == childName) {
            continue;
        }
        for (auto &item : ty->GetSuperInterfaceTys()) {
            if (item->name == type->name) {
                diagEngine->Diagnose(inheritableDecl.begin, inheritableDecl.end,
                    CodeCheckDiagKind::G_ITF_03_avoid_declaring_both_parent_interface_and_sub_interface, childName,
                    type->name);
            } else {
                if (tySet.count(item) > 0) {
                    continue;
                }
                tySet.insert(item);
                CheckParent(childName, item, inheritableDecl, tySet);
            }
        }
    }
}

void StructuralRuleGITF03::CheckInheritedTypes(const InheritableDecl &inheritableDecl)
{
    for (auto &type : inheritableDecl.GetSuperInterfaceTys()) {
        CheckParent(type->name, type, inheritableDecl);
    }
}

void StructuralRuleGITF03::FindTypeDecl(Node *node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Node *node) -> VisitAction {
        return match(*node)(
            [this](const InterfaceDecl &interfaceDecl) {
                interfaceDecl.GetSuperInterfaceTys();
                CheckInheritedTypes(interfaceDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ClassDecl &classDecl) {
                CheckInheritedTypes(classDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const StructDecl &structDecl) {
                CheckInheritedTypes(structDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ExtendDecl &extendDecl) {
                CheckInheritedTypes(extendDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const EnumDecl &enumDecl) {
                CheckInheritedTypes(enumDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGITF03::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindTypeDecl(node);
}
}
