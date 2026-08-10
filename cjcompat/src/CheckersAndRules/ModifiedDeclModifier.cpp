// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

bool CheckerImpl::VarModifierModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(
        v1, diff.GetDomPotentiallyModified(), dsl.VarLetOrConst(v1) && dsl.TopLevel(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()), dsl.VarLetOrConst(v2) && dsl.TopLevel(v2))
        CHECK(RuleKind::VAR_NON_ACCESS_MODIFIER_MODIFIED,
            dsl.IsLet(v1) || dsl.IsVar(v1) && dsl.IsVar(v2) || dsl.IsConst(v1) && dsl.IsConst(v2), v1, v2);
        CHECK(RuleKind::VAR_NON_ACCESS_MODIFIER_MODIFIED_LET_CONST, !(dsl.IsLet(v1) && dsl.IsConst(v2)), v1, v2);
        CHECK(RuleKind::VAR_ACCESS_MODIFIER_MODIFIED, !(diff.ModuleVisible(v1) && !diff.ModuleVisible(v2)), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::FuncModifierModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.Func(f1) && dsl.TopLevel(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()), dsl.Func(f2) && dsl.TopLevel(f2));
        CHECK(RuleKind::FUNC_ACCESS_MODIFIER_MODIFIED, !(diff.ModuleVisible(f1) && !diff.ModuleVisible(f2)), f1, f2);
        if (diff.ModuleVisible(f1) && diff.ModuleVisible(f2)) {
            CHECK(RuleKind::FUNC_NON_ACCESS_MODIFIER_UNSAFE_ADDED,
                f1->TestAttr(Attribute::UNSAFE) || !f2->TestAttr(Attribute::UNSAFE), f1, f2);
            CHECK(RuleKind::FUNC_NON_ACCESS_MODIFIER_CONST_REMOVED, !dsl.IsConst(f1) || dsl.IsConst(f2), f1, f2);
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructModifierModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDomPotentiallyModified(), dsl.StructDecl(s1) && dsl.TopLevel(s1))
        LETIF(s2, dsl.Corresponding(s1, diff.PotentiallyModified()), dsl.StructDecl(s2) && dsl.TopLevel(s2))
        CHECK(RuleKind::STRUCT_ACCESS_MODIFIER_MODIFIED, !(diff.ModuleVisible(s1) && !diff.ModuleVisible(s2)), s1, s2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumModifierModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(e1) && dsl.TopLevel(e1));
        LETIF(e2, dsl.Corresponding(e1, diff.PotentiallyModified()), dsl.EnumDecl(e2) && dsl.TopLevel(e2))
        CHECK(RuleKind::ENUM_ACCESS_MODIFIER_MODIFIED, !(diff.ModuleVisible(e1) && !diff.ModuleVisible(e2)), e1, e2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassModifierModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(c1)
            && diff.ModuleVisible(c1) && dsl.TopLevel(c1));
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.ClassDecl(c2) && dsl.TopLevel(c2))
        CHECK(RuleKind::CLASS_MODIFIER_SEALED_DELETED, !(c1->TestAttr(Attribute::SEALED) &&
            !c2->TestAttr(Attribute::SEALED) && !c2->TestAttr(Attribute::PUBLIC)), c1, c2);
        CHECK(RuleKind::CLASS_MODIFIER_PUBLIC_DELETED,
            !(c1->TestAttr(Attribute::PUBLIC) && !c1->TestAttr(Attribute::SEALED)
                && !c2->TestAttr(Attribute::PUBLIC)), c1, c2);
        CHECK(RuleKind::CLASS_NON_ACCESS_MODIFIER_ABSTRACT_ADDED,
            !(!c1->TestAttr(Attribute::ABSTRACT) && c2->TestAttr(Attribute::ABSTRACT)), c1, c2);
        CHECK(RuleKind::CLASS_NON_ACCESS_MODIFIER_ABSTRACT_DELETED,
            !(c1->TestAttr(Attribute::ABSTRACT) && !c2->TestAttr(Attribute::ABSTRACT)), c1, c2);
        CHECK(RuleKind::CLASS_NON_ACCESS_MODIFIER_SEALED_ADDED,
            c1->TestAttr(Attribute::SEALED) || c1->TestAttr(Attribute::OPEN) || !c2->TestAttr(Attribute::SEALED) ||
                !c2->TestAttr(Attribute::OPEN),
            c1, c2);
        CHECK(RuleKind::CLASS_NON_ACCESS_MODIFIER_OPEN_REMOVED,
            !c1->TestAttr(Attribute::OPEN) || c1->TestAttr(Attribute::SEALED) || c2->TestAttr(Attribute::OPEN), c1, c2);

    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceModifierModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(i1, diff.GetDomPotentiallyModified(),
        dsl.InterfaceDecl(i1) && dsl.TopLevel(i1) && diff.ModuleVisible(i1));
        LETIF(i2, dsl.Corresponding(i1, diff.PotentiallyModified()), dsl.InterfaceDecl(i2) && dsl.TopLevel(i2))
        CHECK(RuleKind::INTERFACE_MODIFIER_SEALED_ADDED,
            !(!i1->TestAttr(Attribute::SEALED) && i2->TestAttr(Attribute::SEALED)),
            i1, i2);
        CHECK(RuleKind::INTERFACE_MODIFIER_SEALED_DELETED,
            !(i1->TestAttr(Attribute::SEALED) && !i2->TestAttr(Attribute::SEALED) && !i2->TestAttr(Attribute::PUBLIC)),
            i1, i2);
        CHECK(RuleKind::INTERFACE_MODIFIER_PUBLIC_DELETED,
            !(i1->TestAttr(Attribute::PUBLIC) && !i1->TestAttr(Attribute::SEALED) && !i2->TestAttr(Attribute::PUBLIC)),
            i1, i2);
    END_FORALL()
    return checkerResult;
}