// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

bool CheckerImpl::InterfaceStructImplDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.StructDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.StructDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        auto x = dsl.GetInheritedTypesSize(f1, dsl.IsInterface) > dsl.GetInheritedTypesSize(f2, dsl.IsInterface);
        CHECK(RuleKind::INTERFACE_STRUCT_IMPL_DELETED, !x, f1);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceStructImplChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.StructDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.StructDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        if (dsl.GetInheritedTypesSize(f1, dsl.IsInterface) > dsl.GetInheritedTypesSize(f2, dsl.IsInterface)) {
            continue;
        }
        auto newInterfaces = dsl.GetInheritedTypes(f2, dsl.IsInterface);
        BEGIN_FORALL(i1, dsl.GetInheritedTypes(f1, dsl.IsInterface), true)
            CHECK(RuleKind::INTERFACE_STRUCT_IMPL_CHANGED, newInterfaces.count(i1) != 0, f1, f2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceEnumImplDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.EnumDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        auto x = dsl.GetInheritedTypesSize(f1, dsl.IsInterface) > dsl.GetInheritedTypesSize(f2, dsl.IsInterface);
        CHECK(RuleKind::INTERFACE_ENUM_IMPL_DELETED, !x, f1);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceEnumImplChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.EnumDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        if (dsl.GetInheritedTypesSize(f1, dsl.IsInterface) > dsl.GetInheritedTypesSize(f2, dsl.IsInterface)) {
            continue;
        }
        auto newInterfaces = dsl.GetInheritedTypes(f2, dsl.IsInterface);
        BEGIN_FORALL(i1, dsl.GetInheritedTypes(f1, dsl.IsInterface), true)
            CHECK(RuleKind::INTERFACE_ENUM_IMPL_CHANGED, newInterfaces.count(i1) != 0, f1, f2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceClassImplAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.ClassDecl(c2) && diff.ModuleVisible(c2))
        CHECK(RuleKind::INTERFACE_CLASS_IMPL_ADDED, !dsl.HasUnimplementedInterface(c1, c2), c1, c2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceClassImplDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.ClassDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        auto x = dsl.GetInheritedTypesSize(f1, dsl.IsInterface) > dsl.GetInheritedTypesSize(f2, dsl.IsInterface);
        CHECK(RuleKind::INTERFACE_CLASS_IMPL_DELETED, !x, f1);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceClassImplChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.ClassDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        if (dsl.GetInheritedTypesSize(f1, dsl.IsInterface) > dsl.GetInheritedTypesSize(f2, dsl.IsInterface)) {
            continue;
        }
        auto newInterfaces = dsl.GetInheritedTypes(f2, dsl.IsInterface);
        BEGIN_FORALL(i1, dsl.GetInheritedTypes(f1, dsl.IsInterface), true)
            CHECK(RuleKind::INTERFACE_CLASS_IMPL_CHANGED, newInterfaces.count(i1) != 0, f1, f2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceInterfaceImplAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(i1, diff.GetDomPotentiallyModified(), dsl.InterfaceDecl(i1) && diff.ModuleVisible(i1))
        LETIF(i2, dsl.Corresponding(i1, diff.PotentiallyModified()), dsl.InterfaceDecl(i2) && diff.ModuleVisible(i2))
        CHECK(RuleKind::INTERFACE_INTERFACE_IMPL_ADDED, !dsl.HasUnimplementedInterface(i1, i2), i1, i2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceInterfaceImplDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(),
        dsl.InterfaceDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.InterfaceDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        auto x = dsl.GetInheritedTypesSize(f1, dsl.IsInterface) > dsl.GetInheritedTypesSize(f2, dsl.IsInterface);
        CHECK(RuleKind::INTERFACE_INTERFACE_IMPL_DELETED, !x, f1);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceInterfaceImplChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(),
        dsl.InterfaceDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.InterfaceDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        if (dsl.GetInheritedTypesSize(f1, dsl.IsInterface) > dsl.GetInheritedTypesSize(f2, dsl.IsInterface)) {
            continue;
        }
        auto newInterfaces = dsl.GetInheritedTypes(f2, dsl.IsInterface);
        BEGIN_FORALL(i1, dsl.GetInheritedTypes(f1, dsl.IsInterface), true)
            CHECK(RuleKind::INTERFACE_INTERFACE_IMPL_CHANGED, newInterfaces.count(i1) != 0, f1, f2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::SuperclassClassInheritDeletedOrAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.ClassDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        auto size1 = dsl.GetInheritedTypesSize(f1, dsl.IsClass);
        auto size2 = dsl.GetInheritedTypesSize(f2, dsl.IsClass);
        auto x = size1 > size2;
        auto y = size1 < size2;
        CHECK(RuleKind::SUPERCLASS_CLASS_INHERIT_DELETED, !x, f1);
        CHECK(RuleKind::SUPERCLASS_CLASS_INHERIT_ADDED, !y, f1);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::SuperclassClassInheritChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()),
            dsl.ClassDecl(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2))
        if (dsl.GetInheritedTypesSize(f1, dsl.IsClass) > dsl.GetInheritedTypesSize(f2, dsl.IsClass)) {
            continue;
        }
        auto newSuperClasses = dsl.GetInheritedTypes(f2, dsl.IsClass);
        BEGIN_FORALL(i1, dsl.GetInheritedTypes(f1, dsl.IsClass), true)
            CHECK(RuleKind::SUPERCLASS_CLASS_INHERIT_CHANGED, newSuperClasses.count(i1) != 0, f1, f2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceExtendImplDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDeleted(), dsl.ExtendDecl(e1) && diff.ModuleVisible(e1));
        BEGIN_FORALL(e2, diff.GetAdded(),
            dsl.ExtendDecl(e2) && diff.ModuleVisible(e2) && diff.SameExtend(e1, e2));
            auto x = dsl.GetInheritedTypesFromExtendDecl(e1).size() >
                    dsl.GetInheritedTypesFromExtendDecl(e2).size();
            CHECK(RuleKind::INTERFACE_EXTEND_IMPL_DELETED, !x, e1, e2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceExtendImplChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDeleted(), dsl.ExtendDecl(e1) && diff.ModuleVisible(e1));
        BEGIN_FORALL(e2, diff.GetAdded(),
            dsl.ExtendDecl(e2) && diff.ModuleVisible(e2) && diff.SameExtend(e1, e2));
            auto inheritedTypes1 = dsl.GetInheritedTypesFromExtendDecl(e1);
            auto inheritedTypes2 = dsl.GetInheritedTypesFromExtendDecl(e2);
            if (inheritedTypes1.size() > inheritedTypes2.size()) {
                continue;
            }
            BEGIN_FORALL(i1, inheritedTypes1, true)
                CHECK(RuleKind::INTERFACE_EXTEND_IMPL_CHANGED, inheritedTypes2.count(i1) != 0, e1, e2);
            END_FORALL()
        END_FORALL()
    END_FORALL()
    return checkerResult;
}