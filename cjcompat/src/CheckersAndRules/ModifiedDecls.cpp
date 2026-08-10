// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

namespace {
static std::vector<Ptr<Decl>> GetEnumConstructors(Node* n)
{
    std::vector<Ptr<Decl>> res;
    if (!n || n->astKind != ASTKind::ENUM_DECL) {
        return res;
    }
    auto ed = static_cast<AST::EnumDecl*>(n);
    std::copy_if(ed->constructors.begin(), ed->constructors.end(),
        std::back_inserter(res), [](auto& arg) { return true; });
    return res;
}
}

bool CheckerImpl::VarDepreAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(
        v1, diff.GetDomPotentiallyModified(), dsl.VarLetOrConst(v1) && dsl.TopLevel(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()),
            dsl.VarLetOrConst(v2) && dsl.TopLevel(v2) && diff.ModuleVisible(v2))
        auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(v1);
        auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(v2);
        CHECK(RuleKind::VAR_DEPRE_ANNO_MOD, !(v1DeprecatedAnno && v2DeprecatedAnno && !v1Strict && v2Strict), v1, v2);
        CHECK(RuleKind::VAR_DEPRE_ANNO_ADD, !(!v1DeprecatedAnno && v2DeprecatedAnno && v2Strict), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::FuncDepreAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(), dsl.Func(v1) && dsl.TopLevel(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()),
            dsl.Func(v2) && dsl.TopLevel(v2) && diff.ModuleVisible(v2))
        auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(v1);
        auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(v2);
        CHECK(RuleKind::FUNC_DEPRE_ANNO_MOD, !(v1DeprecatedAnno && v2DeprecatedAnno && !v1Strict && v2Strict), v1, v2);
        CHECK(RuleKind::FUNC_DEPRE_ANNO_ADD, !(!v1DeprecatedAnno && v2DeprecatedAnno && v2Strict), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructDepreAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(), dsl.StructDecl(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()), dsl.StructDecl(v2))
        auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(v1);
        auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(v2);
        CHECK(
            RuleKind::STRUCT_DEPRE_ANNO_MOD, !(v1DeprecatedAnno && v2DeprecatedAnno && !v1Strict && v2Strict), v1, v2);
        CHECK(RuleKind::STRUCT_DEPRE_ANNO_ADD, !(!v1DeprecatedAnno && v2DeprecatedAnno && v2Strict), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumDepreAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()), dsl.EnumDecl(v2))
        auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(v1);
        auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(v2);
        CHECK(RuleKind::ENUM_DEPRE_ANNO_MOD, !(v1DeprecatedAnno && v2DeprecatedAnno && !v1Strict && v2Strict), v1, v2);
        CHECK(RuleKind::ENUM_DEPRE_ANNO_ADD, !(!v1DeprecatedAnno && v2DeprecatedAnno && v2Strict), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassDepreAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()), dsl.ClassDecl(v2))
        auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(v1);
        auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(v2);
        CHECK(RuleKind::CLASS_DEPRE_ANNO_MOD, !(v1DeprecatedAnno && v2DeprecatedAnno && !v1Strict && v2Strict), v1, v2);
        CHECK(RuleKind::CLASS_DEPRE_ANNO_ADD, !(!v1DeprecatedAnno && v2DeprecatedAnno && v2Strict), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceDepreAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(), dsl.InterfaceDecl(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()), dsl.InterfaceDecl(v2))
        auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(v1);
        auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(v2);
        CHECK(RuleKind::INTERFACE_DEPRE_ANNO_MOD, !(v1DeprecatedAnno && v2DeprecatedAnno && !v1Strict && v2Strict), v1,
            v2);
        CHECK(RuleKind::INTERFACE_DEPRE_ANNO_ADD, !(!v1DeprecatedAnno && v2DeprecatedAnno && v2Strict), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::FuncCAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(), dsl.Func(v1) && dsl.TopLevel(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()),
            dsl.Func(v2) && dsl.TopLevel(v2) && diff.ModuleVisible(v2))
        auto [v1CAttr, v1Unsafe] = dsl.CheckCAnnotation(v1);
        auto [v2CAttr, v2Unsafe] = dsl.CheckCAnnotation(v2);
        CHECK(RuleKind::FUNC_C_ANNO_ADD, !(!v1CAttr && v2CAttr && !v2Unsafe), v1, v2);
        CHECK(RuleKind::FUNC_C_ANNO_DEL, !(v1CAttr && !v2CAttr), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructCAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(), dsl.StructDecl(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()), dsl.StructDecl(v2))
        auto [v1CAttr, v1Unsafe] = dsl.CheckCAnnotation(v1);
        auto [v2CAttr, v2Unsafe] = dsl.CheckCAnnotation(v2);
        CHECK(RuleKind::STRUCT_C_ANNO_ADD, !(!v1CAttr && v2CAttr), v1, v2);
        CHECK(RuleKind::STRUCT_C_ANNO_DEL, !(v1CAttr && !v2CAttr), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::FuncCallingConvAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(),
        dsl.Func(v1) && dsl.TopLevel(v1) && (diff.ModuleVisible(v1) || dsl.IsForeign(v1)))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()),
            dsl.Func(v2) && dsl.TopLevel(v2) && (diff.ModuleVisible(v2) || dsl.IsForeign(v2)))
        auto [v1Cdecl, v1Stdcall] = dsl.CheckCallingConvAnnotation(v1);
        auto [v2Cdecl, v2Stdcall] = dsl.CheckCallingConvAnnotation(v2);
        CHECK(RuleKind::FUNC_CALLINGCONV_ANNO, ((v1Cdecl == v2Cdecl) && (v1Stdcall == v2Stdcall)), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::FuncFrozenAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(), dsl.Func(v1) && dsl.TopLevel(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()),
            dsl.Func(v2) && dsl.TopLevel(v2) && diff.ModuleVisible(v2))
        auto v1Frozendecl = dsl.IsFrozen(v1);
        auto v2Frozendecl = dsl.IsFrozen(v2);
        CHECK(RuleKind::FUNC_FROZEN_ANNO, !(v1Frozendecl && !v2Frozendecl), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::VarGlobalType(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(
        v1, diff.GetDomPotentiallyModified(), dsl.VarLetOrConst(v1) && dsl.TopLevel(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()), dsl.VarLetOrConst(v2) && dsl.TopLevel(v2))
        auto t1 = v1->GetTy().get();
        auto t2 = v2->GetTy().get();
        auto sameClassLikeType = t1->IsClassLike() && t2->IsClassLike();
        // When B is a subtype of A, AND both A and B are of the class or interface type, the two types are compatible.
        // Otherwise, it is not compatible.
        if (!dsl.SameType(t1, t2)) {
            auto isSubtype = dsl.IsParentType(t1, t2);
            CHECK(RuleKind::VAR_GLOBAL_TYPE_MOD, isSubtype, v1, v2);
            CHECK(RuleKind::VAR_GLOBAL_TYPE_MOD_CLASS_LIKE, !isSubtype || sameClassLikeType, v1, v2);
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::VarGlobalInit(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(v1, diff.GetDomPotentiallyModified(),
        dsl.VarLetOrConst(v1) && dsl.TopLevel(v1) && dsl.IsConst(v1) && dsl.IsInitialized(v1) && diff.ModuleVisible(v1))
        LETIF(v2, dsl.Corresponding(v1, diff.PotentiallyModified()),
            dsl.VarLetOrConst(v2) && dsl.TopLevel(v2) && dsl.IsConst(v2) && dsl.IsInitialized(v2))
        CHECK(RuleKind::VAR_GLOBAL_INIT, dsl.SameVarValue(v1, v2), v1, v2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ConstOrFrozenBodyChange(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.Func(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        LETIF(f2, dsl.Corresponding(f1, diff.PotentiallyModified()), dsl.IsConstOrFrozenFunc(f1, f2))
        CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, Dsl::BodyHashOf(f1) == Dsl::BodyHashOf(f2), f1, f2);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumConstructorDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(e1) && diff.ModuleVisible(e1))
        BEGIN_FORALL(con1, diff.GetMemberDeleted(e1), con1->TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR))
            CHECK(RuleKind::ENUM_CONSTRUCTOR_DELETED, false, con1);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumConstructorAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(e1) && diff.ModuleVisible(e1))
        LETIF(e2, dsl.Corresponding(e1, diff.PotentiallyModified()), dsl.EnumDecl(e2) && diff.ModuleVisible(e2))

        auto e1Constructors = GetEnumConstructors(e1);
        auto e2Constructors = GetEnumConstructors(e2);

        // Check if all original constructors did not have associated values
        bool IsE1AllVar = true;
        BEGIN_FORALL(con1, e1Constructors, dsl.Func(con1))
            IsE1AllVar = false;
        END_FORALL()

        auto isNonExhaustive =
            static_cast<AST::EnumDecl*>(e1)->hasEllipsis && static_cast<AST::EnumDecl*>(e2)->hasEllipsis;
        auto thresholdExceeded = static_cast<AST::EnumDecl*>(e1)->constructors.size() <= 256 &&
            static_cast<AST::EnumDecl*>(e2)->constructors.size() > 256;
        CHECK(RuleKind::ENUM_CONSTRUCTOR_ADDED_NON_EXHAUSTIVE, !thresholdExceeded, e1, e2);
        BEGIN_FORALL(con1, diff.GetMemberAdded(e1), con1->TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR))
            auto modifiedMemoryLayout = IsE1AllVar && dsl.Func(con1);
            CHECK(RuleKind::ENUM_CONSTRUCTOR_ADDED_EXHAUSTIVE, isNonExhaustive, con1);
            CHECK(RuleKind::ENUM_CONSTRUCTOR_ADDED_NON_EXHAUSTIVE,
                !(isNonExhaustive &&
                    (modifiedMemoryLayout || !dsl.IsAtEnd(con1, diff.GetMemberAdded(e1), e2Constructors))),
                con1);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumInstMemberVarModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(e1) && diff.ModuleVisible(e1))
        LETIF(e2, dsl.Corresponding(e1, diff.PotentiallyModified()), dsl.EnumDecl(e1) && diff.ModuleVisible(e1))
        auto e1Constructors = GetEnumConstructors(e1);
        auto e2Constructors = GetEnumConstructors(e2);
        LETIF(v1, e1Constructors.begin(), true)
        LETIF(v2, e2Constructors.begin(), true)
        for (; v1 != e1Constructors.end() && v2 != e2Constructors.end(); ++v1, ++v2) {
            LETIF(v1Corsp, dsl.Corresponding(*v1, diff.GetPotentiallyMemberModified(e1)), v1Corsp != nullptr)
            CHECK(RuleKind::ENUM_MEMBER_VAR_ORDER_MODIFIED,
                ((*v1)->mangledName == (*v2)->mangledName) || !dsl.SameType((*v1)->GetTy(), v1Corsp->GetTy()), *v1, *v2);
        }
    END_FORALL()
    return checkerResult;
}
