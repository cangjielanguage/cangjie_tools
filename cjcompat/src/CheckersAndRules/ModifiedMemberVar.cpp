// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

namespace {
static std::vector<Ptr<Decl>> GetMemberVars(Node* n, bool onlyNonStatic = false)
{
    std::vector<Ptr<Decl>> res;
    auto pdecl = dynamic_cast<Decl*>(n);
    if (!pdecl) {
        return res;
    }
    auto& memberDecls = pdecl->GetMemberDecls();
    std::copy_if(memberDecls.begin(), memberDecls.end(),
        std::back_inserter(res), [&](auto& d) {
            return d->astKind == ASTKind::VAR_DECL && !(onlyNonStatic && d->TestAttr(Attribute::STATIC));
        });
    return res;
}
}

bool CheckerImpl::StructMemberVarModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDomPotentiallyModified(),
        dsl.StructDecl(s1) && dsl.TopLevel(s1) && diff.ModuleVisible(s1))
        LETIF(s2, dsl.Corresponding(s1, diff.PotentiallyModified()), dsl.TopLevel(s2))
        BEGIN_FORALL(
            v1, diff.GetDomPotentiallyMemberModified(s1), dsl.VarLetOrConst(v1) && !v1->TestAttr(Attribute::STATIC))
            LETIF(v2, dsl.Corresponding(v1, diff.GetPotentiallyMemberModified(s1)),
                dsl.VarLetOrConst(v2) && !v2->TestAttr(Attribute::STATIC))
            if (diff.ModuleVisible(v1)) {
                CHECK(RuleKind::STRUCT_MEMBER_VAR_NON_ACCESS_MODIFIER_MODIFIED,
                    (dsl.IsLet(v1) && (dsl.IsLet(v2) || dsl.IsVar(v2))) || dsl.IsVar(v1) && dsl.IsVar(v2) ||
                        dsl.IsConst(v1) && dsl.IsConst(v2), v1, v2);
            }
            if (dsl.IsPublic(v1) && dsl.IsPublic(v2)) {
                CHECK(RuleKind::STRUCT_MEMBER_VAR_TYPE_MODIFIED, dsl.SameType(v1->GetTy(), v2->GetTy()), v1, v2);
            }
            if (!dsl.IsPublic(v1) && !dsl.IsPublic(v2)) {
                CHECK(RuleKind::STRUCT_NONPUBLIC_MEMBER_VAR_TYPE_MODIFIED, dsl.SameType(v1->GetTy(), v2->GetTy()), v1, v2);
            }
            CHECK(RuleKind::STRUCT_INSTANCE_VAR_ACCESS_MODIFIED,
                !(dsl.IsPublic(v1) && !dsl.IsPublic(v2)), v1, v2);
            auto modifyInitValue = !dsl.SameVarValue(v1, v2) && dsl.HasConstOrFrozenInit(s1)
                && dsl.HasConstOrFrozenInit(s2) && !dsl.IsNamedParam(v1) && !dsl.IsMemberParam(v1);
            CHECK(RuleKind::STRUCT_CLASS_MEMBER_VAR_INIT_MODIFIED, !modifyInitValue, v1, v2);
        END_FORALL()

        auto s1Vars = GetMemberVars(s1);
        auto s2Vars = GetMemberVars(s2);
        LETIF(v1, s1Vars.begin(), true)
        LETIF(v2, s2Vars.begin(), true)

        if (s1Vars.size() == s2Vars.size()) {
            for (; v1 != s1Vars.end() && v2 != s2Vars.end(); ++v1, ++v2) {
                LETIF(v1Corsp, dsl.Corresponding(*v1, diff.GetPotentiallyMemberModified(s1)), true)
                CHECK(RuleKind::STRUCT_VAR_MODIFIER_STATIC_DELETED,
                    (!((*v1)->TestAttr(Attribute::STATIC) && !(*v2)->TestAttr(Attribute::STATIC))), *v1, *v2);
                if (!(*v1)->TestAttr(Attribute::STATIC) && !(*v2)->TestAttr(Attribute::STATIC)) {
                    CHECK(RuleKind::STRUCT_MEMBER_VAR_ORDER_MODIFIED,
                        ((*v1)->mangledName == (*v2)->mangledName) || v1Corsp == nullptr ||
                            !dsl.SameType((*v1)->GetTy(), v1Corsp->GetTy()),
                        *v1, *v2);
                }
            }
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructVarDepreAnno(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.StructDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.StructDecl(c1) && dsl.TopLevel(c1))
        BEGIN_FORALL(v1, diff.GetDomPotentiallyMemberModified(c1), dsl.VarLetOrConst(v1) && diff.ModuleVisible(v1))
            LETIF(v2, dsl.Corresponding(v1, diff.GetPotentiallyMemberModified(c1)),
                dsl.VarLetOrConst(v2) && diff.ModuleVisible(v2))
            auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(v1);
            auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(v2);
            CHECK(RuleKind::STRUCT_VAR_DEPRE_ANNO_MOD, !(v1DeprecatedAnno && v2DeprecatedAnno && !v1Strict && v2Strict),
                v1, v2);
            CHECK(RuleKind::STRUCT_VAR_DEPRE_ANNO_ADD, !(!v1DeprecatedAnno && v2DeprecatedAnno && v2Strict), v1, v2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructMemberVarDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDomPotentiallyModified(), dsl.StructDecl(s1) && diff.ModuleVisible(s1) && dsl.TopLevel(s1))
        bool IsSameNumMemberParams = false;
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(s1),
            dsl.Func(f1) && diff.ModuleVisible(f1) && f1->TestAnyAttr(Attribute::CONSTRUCTOR));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(s1));
            auto numParams1 = dsl.NumMemberParams(f1);
            if (numParams1 == 0) {
                continue;
            }
            auto numParams2 = dsl.NumMemberParams(f2);
            if (numParams1 == numParams2) {
                IsSameNumMemberParams = true;
                break;
            }
        END_FORALL()
        LETIF(s2, dsl.Corresponding(s1, diff.PotentiallyModified()), dsl.StructDecl(s2) && dsl.TopLevel(s2))
        if (!dsl.SameNamelessDecls(s1, s2)) {
            BEGIN_FORALL(v1, diff.GetMemberDeleted(s1), dsl.VarLetOrConst(v1))
                CHECK(RuleKind::STRUCT_STATIC_VAR_DEL, !diff.ModuleVisible(v1) || !v1->TestAttr(Attribute::STATIC), v1);
            END_FORALL()
            LETIF(c2, dsl.Corresponding(s1, diff.PotentiallyModified()),
                dsl.StructDecl(c2) && diff.ModuleVisible(c2) && dsl.TopLevel(c2))

            BEGIN_FORALL(v1, diff.GetMemberAdded(s1),
                dsl.VarLetOrConst(v1) && !v1->TestAttr(Attribute::STATIC))
                // Only the primary constructor exists and the number of member variable params is the same,
                // the non-public member variable param should be continue.
                if (!dsl.IsPublic(v1) && dsl.IsMemberParam(v1) && IsSameNumMemberParams) {
                    continue;
                }
                CHECK(RuleKind::STRUCT_MEMBER_VAR_ADDED_ABI, !dsl.VarLetOrConst(v1), v1);
            END_FORALL()
            BEGIN_FORALL(v1, diff.GetMemberDeleted(s1), dsl.VarLetOrConst(v1) && !v1->TestAttr(Attribute::STATIC))
                CHECK(RuleKind::STRUCT_INSTANCE_VAR_DEL, !dsl.IsPublic(v1), v1);
            END_FORALL()
            BEGIN_FORALL(v1, diff.GetMemberDeleted(s1),
                dsl.VarLetOrConst(v1) && !v1->TestAttr(Attribute::STATIC))
                if (!dsl.IsPublic(v1) && dsl.IsMemberParam(v1) && IsSameNumMemberParams) {
                    continue;
                }
                CHECK(RuleKind::STRUCT_INST_NO_PUBLIC_VAR_DEL, dsl.IsPublic(v1), v1);
            END_FORALL()
        }
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructStaticMemberVarModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.StructDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.StructDecl(c2) && dsl.TopLevel(c2))
        BEGIN_FORALL(v1, diff.GetDomPotentiallyMemberModified(c1),
            dsl.VarLetOrConst(v1) && v1->TestAttr(Attribute::STATIC) && diff.ModuleVisible(v1))
            LETIF(v2, dsl.Corresponding(v1, diff.GetPotentiallyMemberModified(c1)),
                dsl.VarLetOrConst(v2) && v2->TestAttr(Attribute::STATIC))
                CHECK(RuleKind::STRUCT_STATIC_VAR_PUBLIC_DEL, !(dsl.IsPublic(v1) && !dsl.IsPublic(v2)), v1, v2);
                if (!diff.ModuleVisible(v2)) {
                    continue;
                }
                auto t1 = v1->GetTy().get();
                auto t2 = v2->GetTy().get();
                CHECK(RuleKind::STRUCT_STATIC_VAR_INIT_MODIFIED,
                    !(!dsl.SameVarValue(v1, v2) && dsl.IsConst(v1) && dsl.IsConst(v2)), v1, v2);
                CHECK(RuleKind::STRUCT_STATIC_MEMBER_VAR_TO_LET, !(dsl.IsVar(v1) && dsl.IsLet(v2)), v1, v2);
                CHECK(RuleKind::STRUCT_STATIC_MEMBER_VAR_TO_CONST, !(dsl.IsVar(v1) && dsl.IsConst(v2)), v1, v2);
                CHECK(RuleKind::STRUCT_STATIC_MEMBER_CONST_TO_LET, !(dsl.IsConst(v1) && dsl.IsLet(v2)), v1, v2);
                CHECK(RuleKind::STRUCT_STATIC_MEMBER_CONST_TO_VAR, !(dsl.IsConst(v1) && dsl.IsVar(v2)), v1, v2);
                CHECK(RuleKind::STRUCT_STATIC_MEMBER_LET_TO_CONST, !(dsl.IsLet(v1) && dsl.IsConst(v2)), v1, v2);
                if (!dsl.SameType(t1, t2)) {
                    auto isSubtype = dsl.IsParentType(t1, t2);
                    CHECK(RuleKind::STRUCT_STATIC_VAR_TYPE_MODIFIED, isSubtype, v1, v2);
                    CHECK(RuleKind::STRUCT_STATIC_VAR_TYPE_MODIFIED_CLASS_LIKE,
                        !isSubtype || (Dsl::IsClassLikeTy(t1) && Dsl::IsClassLikeTy(t2)), v1, v2);
                }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstMemberVarAddedOrDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.ClassDecl(c2))
        if (!dsl.SameNamelessDecls(c1, c2)) {
            // non-public member variable changed.
            auto c2Vars = GetMemberVars(c2, true);
            BEGIN_FORALL(v1, diff.GetMemberAdded(c1), dsl.VarLetOrConst(v1) && !v1->TestAttr(Attribute::STATIC))
                auto c1NotPublicInheritable = c1->TestAttr(Attribute::SEALED) || !diff.ModuleVisible(c1) ||
                    (diff.ModuleVisible(c1) && !c1->TestAttr(Attribute::OPEN));
                auto v1AtEnd = dsl.IsAtEnd(v1, diff.GetMemberAdded(c1), c2Vars);
                CHECK(RuleKind::CLASS_MEMBER_VAR_ADDED_ABI, c1NotPublicInheritable && v1AtEnd, v1);
            END_FORALL()

            BEGIN_FORALL(v1, diff.GetMemberDeleted(c1), dsl.VarLetOrConst(v1))
                auto c1NotPublicInheritable = c1->TestAttr(Attribute::SEALED) || !diff.ModuleVisible(c1) ||
                    (diff.ModuleVisible(c1) && !c1->TestAttr(Attribute::OPEN));
                if (!(v1->TestAttr(Attribute::PRIVATE) || v1->TestAttr(Attribute::INTERNAL))) {
                    continue;
                }
                // ABI Incompatible: delete non-static member variable of class.
                CHECK(RuleKind::CLASS_MEMBER_VAR_DELETED_ABI, v1->TestAttr(Attribute::STATIC), v1);
            END_FORALL()
        }
        // public member variable deleted.
        BEGIN_FORALL(v1, diff.GetMemberDeleted(c1), dsl.VarLetOrConst(v1))
            if (v1->TestAttr(Attribute::STATIC)) {
                CHECK(RuleKind::CLASS_STATIC_MEMBER_VAR_DELETED, !dsl.IsPublicOrProtected(v1), v1);
            } else {
                CHECK(RuleKind::CLASS_PUBLIC_MEMBER_VAR_DELETED, !dsl.IsPublicOrProtected(v1), v1);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstMemberVarModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.ClassDecl(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(v1, diff.GetDomPotentiallyMemberModified(c1),
            dsl.VarLetOrConst(v1) && (diff.ModuleVisible(v1) || v1->TestAttr(Attribute::PROTECTED)))
            LETIF(v2, dsl.Corresponding(v1, diff.GetPotentiallyMemberModified(c1)), dsl.VarLetOrConst(v2))
            auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(v1);
            auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(v2);
            CHECK(RuleKind::CLASS_MEMBER_VAR_DEPRE_ANNO_MOD,
                !(v1DeprecatedAnno && v2DeprecatedAnno && !v1Strict && v2Strict), v1, v2);
            CHECK(RuleKind::CLASS_MEMBER_VAR_DEPRE_ANNO_ADD, !(!v1DeprecatedAnno && v2DeprecatedAnno && v2Strict), v1,
                v2);
        END_FORALL()

        BEGIN_FORALL(v1, diff.GetDomPotentiallyMemberModified(c1),
            dsl.VarLetOrConst(v1) && !v1->TestAttr(Attribute::STATIC) &&
                (diff.ModuleVisible(v1) || v1->TestAttr(Attribute::PROTECTED)))
            LETIF(v2, dsl.Corresponding(v1, diff.GetPotentiallyMemberModified(c1)),
                dsl.VarLetOrConst(v2) && !v2->TestAttr(Attribute::STATIC))
            CHECK(RuleKind::CLASS_MEMBER_VAR_NON_ACCESS_MODIFIER_MODIFIED,
                (dsl.IsLet(v1) && (dsl.IsLet(v2) || dsl.IsVar(v2))) || dsl.IsVar(v1) && dsl.IsVar(v2) ||
                    dsl.IsConst(v1) && dsl.IsConst(v2),
                v1, v2);
            CHECK(RuleKind::CLASS_MEMBER_VAR_ACCESS_MODIFIER_MODIFIED,
                diff.ModuleVisible(v2) ||
                    (v1->TestAttr(Attribute::PROTECTED) &&
                        (v2->TestAttr(Attribute::PROTECTED) || v2->TestAttr(Attribute::PUBLIC))),
                v1, v2);
        END_FORALL()

        auto c1Vars = GetMemberVars(c1, true);
        auto c2Vars = GetMemberVars(c2, true);
        LETIF(v1, c1Vars.begin(), true)
        LETIF(v2, c2Vars.begin(), true)

        for (; v1 != c1Vars.end() && v2 != c2Vars.end(); ++v1, ++v2) {
            LETIF(v1Corsp, dsl.Corresponding(*v1, diff.GetPotentiallyMemberModified(c1)), v1Corsp != nullptr)
            auto modifyVarType = !dsl.SameType((*v1)->GetTy(), v1Corsp->GetTy());
            if (dsl.IsPublicOrProtected(*v1) && dsl.IsPublicOrProtected(v1Corsp)) {
                CHECK(RuleKind::CLASS_PUBLIC_PROTECTED_MEMBER_VAR_TYPE_MODIFIED, !modifyVarType, *v1, v1Corsp);
            }
            else {
                CHECK(RuleKind::CLASS_PRIVATE_INTERNAL_MEMBER_VAR_TYPE_MODIFIED, !modifyVarType, *v1, v1Corsp);
            }
            CHECK(RuleKind::CLASS_MEMBER_VAR_ORDER_MODIFIED,
                ((*v1)->mangledName == (*v2)->mangledName) || !dsl.SameType((*v1)->GetTy(), v1Corsp->GetTy()), *v1, *v2);
        }

        BEGIN_FORALL(v1, diff.GetDomPotentiallyMemberModified(c1),
            dsl.VarLetOrConst(v1) && !v1->TestAttr(Attribute::STATIC))
            LETIF(v2, dsl.Corresponding(v1, diff.GetPotentiallyMemberModified(c1)),
                dsl.VarLetOrConst(v2) && !v2->TestAttr(Attribute::STATIC))
            auto modifyInitValue = !dsl.SameVarValue(v1, v2) && dsl.HasConstOrFrozenInit(c1)
                && dsl.HasConstOrFrozenInit(c2) && !dsl.IsNamedParam(v1) && !dsl.IsMemberParam(v1);
            CHECK(RuleKind::STRUCT_CLASS_MEMBER_VAR_INIT_MODIFIED, !modifyInitValue, v1, v2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassStaticMemberVarModified(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.ClassDecl(c2) && dsl.TopLevel(c2))
        BEGIN_FORALL(
            v1, diff.GetDomPotentiallyMemberModified(c1), dsl.VarLetOrConst(v1) && v1->TestAttr(Attribute::STATIC))
            LETIF(v2, dsl.Corresponding(v1, diff.GetPotentiallyMemberModified(c1)),
                dsl.VarLetOrConst(v2))
            if ((diff.ModuleVisible(v1) || v1->TestAttr(Attribute::PROTECTED)) &&
                (diff.ModuleVisible(v2) || v2->TestAttr(Attribute::PROTECTED))) {
                CHECK(RuleKind::CLASS_STATIC_CONST_VAR_INIT_MODIFIED,
                    !(dsl.IsConst(v1) && dsl.IsConst(v2) && !dsl.SameVarValue(v1, v2)), v1, v2);
            }
            if (diff.ModuleVisible(v1) && diff.ModuleVisible(v2)) {
                CHECK(RuleKind::CLASS_STATIC_VAR_DELETED_STATIC,
                    !(v1->TestAttr(Attribute::STATIC) && !v2->TestAttr(Attribute::STATIC)), v1, v2);
                CHECK(RuleKind::CLASS_STATIC_VAR_NON_ACCESS_MODIFIER_MODIFIED,
                    dsl.IsLet(v1) || dsl.IsVar(v1) && dsl.IsVar(v2) || dsl.IsConst(v1) && dsl.IsConst(v2), v1, v2);
                CHECK(RuleKind::CLASS_STATIC_VAR_NON_ACCESS_MODIFIER_MODIFIED_LET_CONST,
                    !(dsl.IsLet(v1) && dsl.IsConst(v2)), v1, v2);
                auto t1 = v1->GetTy().get();
                auto t2 = v2->GetTy().get();
                if (!dsl.SameType(t1, t2)) {
                    auto isSubtype = dsl.IsParentType(t1, t2);
                    CHECK(RuleKind::CLASS_STATIC_VAR_TYPE_MODIFIED, isSubtype, v1, v2);
                    CHECK(RuleKind::CLASS_STATIC_VAR_TYPE_MODIFIED_CLASS_LIKE,
                        !isSubtype || (Dsl::IsClassLikeTy(t1) && Dsl::IsClassLikeTy(t2)), v1, v2);
                }
            }
            CHECK(RuleKind::CLASS_STATIC_VAR_ACCESS_MODIFIER_MODIFIED,
                !v1->TestAttr(Attribute::PUBLIC) || v2->TestAttr(Attribute::PUBLIC),
                v1, v2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}