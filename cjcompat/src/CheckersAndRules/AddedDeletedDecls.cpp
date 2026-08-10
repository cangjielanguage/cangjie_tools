// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

bool CheckerImpl::VarDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDeleted(), dsl.VarLetOrConst(v1) && diff.ModuleVisible(v1));
        auto hasSameVar = false;
        BEGIN_FORALL(v2, diff.GetAdded(),
            dsl.VarLetOrConst(v2) && diff.ModuleVisible(v2) && dsl.SameIdentifier(v1, v2));
            hasSameVar = true;
            if (v1->TestAttr(Attribute::PRIVATE)) {
                CHECK(RuleKind::VAR_LOCATION_CHANGED, false, v1);
            }
        END_FORALL()
        CHECK(RuleKind::VAR_DELETED, hasSameVar, v1);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDeleted(), dsl.StructDecl(s1) && diff.ModuleVisible(s1));
        auto hasSameStruct = false;
        BEGIN_FORALL(s2, diff.GetAdded(),
            dsl.StructDecl(s2) && diff.ModuleVisible(s2) && dsl.SameIdentifier(s1, s2));
            hasSameStruct = true;
            if (s1->TestAttr(Attribute::PRIVATE)) {
                CHECK(RuleKind::STRUCT_LOCATION_CHANGED, false, s1);
            }
        END_FORALL()
        if (dsl.IsPublic(s1)) {
            CHECK(RuleKind::STRUCT_DELETED, hasSameStruct, s1);
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDeleted(), dsl.EnumDecl(e1) && diff.ModuleVisible(e1));
        auto hasSameEnum = false;
        BEGIN_FORALL(e2, diff.GetAdded(),
            dsl.EnumDecl(e2) && diff.ModuleVisible(e2) && dsl.SameIdentifier(e1, e2));
            hasSameEnum = true;
            if (e1->TestAttr(Attribute::PRIVATE)) {
                CHECK(RuleKind::ENUM_LOCATION_CHANGED, false, e1);
            }
        END_FORALL()
        if (dsl.IsPublic(e1)) {
            CHECK(RuleKind::ENUM_DELETED, hasSameEnum, e1);
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDeleted(), dsl.ClassDecl(c1) && diff.ModuleVisible(c1));
        auto hasSameClass = false;
        BEGIN_FORALL(c2, diff.GetAdded(),
            dsl.ClassDecl(c2) && diff.ModuleVisible(c2) && dsl.SameIdentifier(c1, c2));
            hasSameClass = true;
            if (c1->TestAttr(Attribute::PRIVATE)) {
                CHECK(RuleKind::CLASS_LOCATION_CHANGED, false, c1);
            }
        END_FORALL()
        if (dsl.IsPublic(c1)) {
            CHECK(RuleKind::CLASS_DELETED, hasSameClass, c1);
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(i1, diff.GetDeleted(), dsl.InterfaceDecl(i1) && diff.ModuleVisible(i1));
        auto hasSameInterface = false;
        BEGIN_FORALL(i2, diff.GetAdded(),
            dsl.InterfaceDecl(i2) && diff.ModuleVisible(i2) && dsl.SameIdentifier(i1, i2));
            hasSameInterface = true;
            if (i1->TestAttr(Attribute::PRIVATE)) {
                CHECK(RuleKind::INTERFACE_LOCATION_CHANGED, false, i1);
            }
            CHECK(RuleKind::INTERFACE_MODIFIER_PUBLIC_DELETED,
                !(i1->TestAttr(Attribute::PUBLIC) && !i2->TestAttr(Attribute::PUBLIC)), i1, i2);
        END_FORALL()
        if (dsl.IsPublic(i1)) {
            CHECK(RuleKind::INTERFACE_DELETED, hasSameInterface, i1);
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ExtendDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDeleted(), dsl.ExtendDecl(e1) && diff.ModuleVisible(e1));
        auto hasSameExtend = false;
        BEGIN_FORALL(e2, diff.GetAdded(),
            dsl.ExtendDecl(e2) && diff.ModuleVisible(e2) && diff.SameExtend(e1, e2));
            hasSameExtend = true;
        END_FORALL()
        CHECK(RuleKind::EXTEND_DELETED, hasSameExtend, e1);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::FuncDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDeleted(), dsl.Func(f1) && diff.ModuleVisible(f1));
        auto hasSameFunc = false;
        BEGIN_FORALL(f2, diff.GetAdded(),
            dsl.Func(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2));
            hasSameFunc = true;
            if (f1->TestAttr(Attribute::PRIVATE)) {
                CHECK(RuleKind::FUNC_LOCATION_CHANGED, false, f1);
            }
        END_FORALL()
        if (dsl.IsPublic(f1)) {
            CHECK(RuleKind::FUNC_DELETED, hasSameFunc, f1);
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::AddDelImportStatement(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    std::vector<Node*> checkedNode;
    BEGIN_FORALL(i1, diff.GetDeleted(), dsl.Import(i1));
        bool bModify = false;
        BEGIN_FORALL(i2, diff.GetAdded(), dsl.Import(i2) && !IsCheckedAlready(checkedNode, i2));
            if (dsl.SameImportPackage(i1, i2)) {
                bModify = true;
                checkedNode.emplace_back(i2);
                if (dsl.IsPublic(i1)) {
                    CHECK(RuleKind::DEL_IMPORT_STATEMENT, false, i1);
                    CHECK(RuleKind::ADD_IMPORT_STATEMENT, false, i2);
                }
                break;
            }
        END_FORALL()
        if (dsl.IsPublic(i1)) {
            CHECK(RuleKind::DEL_IMPORT_STATEMENT, bModify, i1);
        } else {
            CHECK(RuleKind::DEL_NONPUBLIC_IMPORT_STATEMENT, bModify, i1);
        }
    END_FORALL()
    BEGIN_FORALL(i2, diff.GetAdded(), dsl.Import(i2) && !IsCheckedAlready(checkedNode, i2));
        CHECK(RuleKind::ADD_IMPORT_STATEMENT, false, i2);
    END_FORALL()
    BEGIN_FORALL(i1, diff.GetDomPotentiallyModified(), dsl.Import(i1) && !dsl.IsPublic(i1))
        auto i2 = dsl.Corresponding(i1, diff.PotentiallyModified());
        if (dsl.IsPublic(i2)) {
            CHECK(RuleKind::ADD_IMPORT_STATEMENT, false, i2);
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::DelModTypeAlias(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(t, diff.GetDeleted(), dsl.TypeAlias(t) && diff.ModuleVisible(t))
        CHECK(RuleKind::DEL_TYPE_ALIAS, false, t);
    END_FORALL()
    BEGIN_FORALL(t, diff.GetDomPotentiallyModified(), dsl.TypeAlias(t) && diff.ModuleVisible(t))
        auto ty1 = dsl.AliasTargetTy(t);
        auto t2 = dsl.Corresponding(t, diff.PotentiallyModified());
        auto ty2 = dsl.AliasTargetTy(t2);
        CHECK(RuleKind::MOD_TYPE_ALIAS, dsl.SameType(ty1, ty2), t);
    END_FORALL()
    return checkerResult;
}
