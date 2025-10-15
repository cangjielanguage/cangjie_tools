// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGCON01.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

void StructuralRuleGCON01::DefaultPackageChecker(const std::set<Cangjie::AST::Modifier> modifiers,
    const Cangjie::Position start, const Cangjie::Position end)
{
    auto result = std::any_of(modifiers.begin(), modifiers.end(),
        [](const Cangjie::AST::Modifier &modifier) { return modifier.modifier == TokenKind::PRIVATE; });
    if (!result) {
        Diagnose(start, end, CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_1, "private");
    }
}

void StructuralRuleGCON01::ClassModifierChecker(const Cangjie::AST::VarDecl &varDecl)
{
    auto result = std::any_of(varDecl.outerDecl->modifiers.begin(), varDecl.outerDecl->modifiers.end(),
        [](const Cangjie::AST::Modifier &modifier) { return modifier.modifier == TokenKind::PUBLIC; });
    if (result) {
        if (varDecl.outerDecl->TestAttr(Attribute::PUBLIC)) {
            if (varDecl.TestAttr(Attribute::PUBLIC)) {
                Diagnose(varDecl.identifier.Begin(), varDecl.identifier.End(),
                    CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_1, "private");
            }
        }

        if (varDecl.outerDecl->TestAttr(Attribute::OPEN) || varDecl.outerDecl->TestAttr(Attribute::ABSTRACT)) {
            if (varDecl.TestAttr(Attribute::PUBLIC) || varDecl.TestAttr(Attribute::PROTECTED)) {
                Diagnose(varDecl.identifier.Begin(), varDecl.identifier.End(),
                    CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_1, "private");
            }
        }
    }
}

void StructuralRuleGCON01::PackageGlobalVarChecker(const std::set<Cangjie::AST::Modifier> modifiers,
    const Cangjie::Position start, const Cangjie::Position end)
{
    auto result = std::any_of(modifiers.begin(), modifiers.end(),
        [](const Cangjie::AST::Modifier &modifier) { return modifier.modifier == TokenKind::PUBLIC; });
    if (result) {
        Diagnose(start, end, CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_0, "public");
    }
}

void StructuralRuleGCON01::SynchronizedObjectFinder(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl &varDecl) {
                if (varDecl.ty == nullptr) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (varDecl.ty->name != "ReentrantMutex") {
                    return VisitAction::SKIP_CHILDREN;
                }

                if (varDecl.fullPackageName == "default") {
                    if (varDecl.TestAttr(Attribute::GLOBAL)) {
                        Diagnose(varDecl.identifier.Begin(), varDecl.identifier.End(),
                            CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_2,
                            varDecl.identifier.Val());
                    }
                    if (varDecl.outerDecl != nullptr && varDecl.outerDecl->IsStructOrClassDecl()) {
                        DefaultPackageChecker(varDecl.modifiers, varDecl.identifier.Begin(), varDecl.identifier.End());
                    }
                } else {
                    if (varDecl.outerDecl != nullptr && varDecl.outerDecl->IsStructOrClassDecl()) {
                        ClassModifierChecker(varDecl);
                    }
                    if (varDecl.TestAttr(Attribute::GLOBAL)) {
                        PackageGlobalVarChecker(
                            varDecl.modifiers, varDecl.identifier.Begin(), varDecl.identifier.End());
                    }
                }

                return VisitAction::WALK_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGCON01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    SynchronizedObjectFinder(node);
}
} // namespace Cangjie::CodeCheck
