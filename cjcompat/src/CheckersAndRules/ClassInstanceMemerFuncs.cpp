// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

namespace {
static std::vector<std::string> GetOpenInstanceMembers(Node* node, Dsl& dsl,
    const std::function<bool(const OwnedPtr<Decl>&)> f)
{
    std::vector<std::string> openFuncs;
    auto x = dynamic_cast<Decl*>(node);
    if (!x) {
        return openFuncs;
    }
    for (auto &member : x->GetMemberDecls()) {
        if (f(member)) {
            openFuncs.emplace_back(dsl.GetMangledName(member.get()));
        }
    }
    return openFuncs;
}

static void CheckClassOpenInstanceMemberOrder(Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult,
    Node* c1, Node* c2, bool checkFunc)
{
    auto predicate = [&dsl, checkFunc] () {
        return [&dsl, checkFunc] (const OwnedPtr<Decl>& x) {
            return (checkFunc ? dsl.Func(x.get()) : dsl.PropDecl(x.get()))
                && !x->TestAnyAttr(Attribute::CONSTRUCTOR)
                && x->TestAnyAttr(Attribute::OPEN, Attribute::ABSTRACT);
        };
    };
    auto oldMembers = GetOpenInstanceMembers(c1, dsl, predicate());
    auto newMembers = GetOpenInstanceMembers(c2, dsl, predicate());
    bool checked = false;
    BEGIN_FORALL(i, dsl.Range(0, oldMembers.size()), oldMembers.size() == newMembers.size())
        CHECK(checkFunc ? RuleKind::CLASS_OPEN_INSTANCE_MEMBER_FUNCS_ORDER_CHANGED :
                          RuleKind::CLASS_OPEN_INSTANCE_MEMBER_PROPS_ORDER_CHANGED,
            !checked || oldMembers[i] == newMembers[i], c1);
        checked = checked || oldMembers[i] != newMembers[i];
    END_FORALL()
}

static Node* GetLastOpenFunc(const Diff& diff, Dsl& dsl, Node* n1)
{
    auto cd = dynamic_cast<ClassDecl*>(n1);
    if (!cd) {
        return nullptr;
    }
    Node* lastOpenFunc = nullptr;
    for (auto &md : cd->GetMemberDecls()) {
        if (!md->TestAttr(Attribute::OPEN) || !md->IsFuncOrProp()) {
            continue;
        }
        auto f2 = dsl.Corresponding(md.get(), diff.GetPotentiallyMemberModified(n1));
        if (f2) {
            lastOpenFunc = f2;
        }
    }
    return lastOpenFunc;
}
}

bool CheckerImpl::ClassMemberFuncOrPropAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetMemberAdded(c1),
            (dsl.Func(f1) || dsl.PropDecl(f1)) && !f1->TestAnyAttr(Attribute::CONSTRUCTOR))
            if (dsl.IsStatic(f1)) {
                if (dsl.Func(f1)) {
                    CHECK(RuleKind::CLASS_STATIC_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(c2, f1), f1);
                }
                if (dsl.PropDecl(f1)) {
                    CHECK(RuleKind::CLASS_STATIC_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(c2, f1), f1);
                }
            } else {
                if (dsl.Func(f1)) {
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_ADDED, !dsl.HasUnimplementedDecl(c2, f1), f1);
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_ADDED2, !dsl.HasOverridingDecl(c2, f1), f1);
                }
                if (dsl.PropDecl(f1)) {
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_ADDED, !dsl.HasUnimplementedDecl(c2, f1), f1);
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_ADDED2, !dsl.HasOverridingDecl(c2, f1), f1);
                }
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassOpenInstanceMemberAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && diff.ModuleVisible(c2))
        // If the added open member function or property is at the end of
        // all open functions and properties, compatible; otherwise, incompatible.
        auto lastOpenFunc = GetLastOpenFunc(diff, dsl, c1);
        BEGIN_FORALL(f1, diff.GetMemberAdded(c1), (dsl.Func(f1) || dsl.PropDecl(f1)) &&
            !f1->TestAnyAttr(Attribute::CONSTRUCTOR) && f1->TestAnyAttr(Attribute::OPEN, Attribute::ABSTRACT) &&
            (lastOpenFunc && f1->begin < lastOpenFunc->begin))
            if (dsl.Func(f1)) {
                CHECK(RuleKind::CLASS_OPEN_INSTANCE_MEMBER_FUNCS_ADDED, false, f1);
            } else if (dsl.PropDecl(f1)) {
                CHECK(RuleKind::CLASS_OPEN_INSTANCE_MEMBER_PROPS_ADDED, false, f1);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassOpenInstanceMemberOrderChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        CheckClassOpenInstanceMemberOrder(dsl, logger, checker, checkerResult, c1, c2, true);
        CheckClassOpenInstanceMemberOrder(dsl, logger, checker, checkerResult, c1, c2, false);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetMemberDeleted(c1), (dsl.Func(f1) || dsl.PropDecl(f1))
            && (dsl.IsPublic(f1) || (f1->TestAnyAttr(Attribute::PROTECTED) && !dsl.IsStatic(f1))))
            if (dsl.PropDecl(f1)) {
                CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROPS_DELETED, false, f1);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberFrozenAnnoDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1),
            (dsl.Func(f1) || dsl.PropDecl(f1)) && diff.ModuleVisible(f1))
            LETIF(f2, dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1)), dsl.Func(f2) || dsl.PropDecl(f2))
            if (dsl.IsFrozen(f1) && !dsl.IsFrozen(f2)) {
                if (dsl.Func(f1)) {
                    if (f1->TestAttr(Attribute::CONSTRUCTOR)) {
                        CHECK(RuleKind::CLASS_CONSTRUCTOR_FROZEN_ANNO_DELETED, false, f1, f2);
                    } else {
                        CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_FROZEN_ANNO_DELETED, false, f1, f2);
                    }
                } else if (dsl.PropDecl(f1)) {
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_FROZEN_ANNO_DELETED, false, f1, f2);
                }
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberVisibilityChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1), dsl.Func(f1))
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1));
            if ((dsl.IsPublicOrProtected(f1) && !dsl.IsPublicOrProtected(f2)) ||
                (dsl.IsPublic(f1) && dsl.IsProtected(f2))) {
                if (f1->TestAttr(Attribute::CONSTRUCTOR)) {
                    CHECK(RuleKind::CLASS_CONSTRUCTOR_VISIBILITY_CHANGED, false, f1);
                } else {
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_VISIBILITY_CHANGED, false, f1);
                }
            }
        END_FORALL()
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(c1), dsl.PropDecl(p1) && diff.ModuleVisible(p1))
            auto p2 = dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(c1));
            if ((dsl.IsPublicOrProtected(p1) && !dsl.IsPublicOrProtected(p2)) ||
                (dsl.IsPublic(p1) && dsl.IsProtected(p2))) {
                CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_VISIBILITY_CHANGED, false, p1);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberFuncUnsafeAttrAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1), dsl.Func(f1) && diff.ModuleVisible(f1))
            LETIF(f2, dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1)), dsl.Func(f2))
            if (!f1->TestAttr(Attribute::CONSTRUCTOR) &&
                !f1->TestAttr(Attribute::UNSAFE) && f2->TestAttr(Attribute::UNSAFE)) {
                CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_UNSAFE_ATTR_ADDED, false, f1, f2);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberFuncConstAttrAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1), dsl.Func(f1) && diff.ModuleVisible(f1))
            LETIF(f2, dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1)), dsl.Func(f2))
            if (!f1->TestAttr(Attribute::STATIC) && !f1->TestAttr(Attribute::CONSTRUCTOR) &&
                !dsl.IsConst(f1) && dsl.IsConst(f2)) {
                CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_CONST_ATTR_ADDED, false, f1, f2);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberFuncConstAttrDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1), dsl.Func(f1) && diff.ModuleVisible(f1))
            LETIF(f2, dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1)), dsl.Func(f2))
            if (dsl.IsConst(f1) && !dsl.IsConst(f2)) {
                if (f1->TestAttr(Attribute::CONSTRUCTOR)) {
                    CHECK(RuleKind::CLASS_CONSTRUCTOR_CONST_ATTR_DELETED, false, f1, f2);
                } else {
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_CONST_ATTR_DELETED, false, f1, f2);
                }
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberFuncStaticAttrAddedDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1));
            CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_STATIC_ATTR_DELETED,
                !(dsl.IsStatic(f1) && !dsl.IsStatic(f2)), f1, f2);
            CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_STATIC_ATTR_ADDED,
                !(!dsl.IsStatic(f1) && dsl.IsStatic(f2)), f1, f2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberFuncOpenAttrAddedDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1));
            CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_OPEN_ATTR_ADDED, !(!dsl.IsOpen(f1) && dsl.IsOpen(f2)), f1, f2);
            CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_OPEN_ATTR_DELETED, !(dsl.IsOpen(f1) && !dsl.IsOpen(f2)), f1, f2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberPropStaticAttrAddedDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(c1),
            dsl.PropDecl(p1) && (dsl.IsPublic(p1) || p1->TestAttr(Attribute::PROTECTED)));
            auto p2 = dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(c1));
            if (!p2) {
                continue;
            }
            if (dsl.IsPublic(p2) || p2->TestAttr(Attribute::PROTECTED)) {
                CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_STATIC_ATTR_ADDED,
                    !(!dsl.IsStatic(p1) && dsl.IsStatic(p2)), p1, p2);
                CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_STATIC_ATTR_DELETED,
                    !(dsl.IsStatic(p1) && !dsl.IsStatic(p2)), p1, p2);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassMemberPropMutAttrDeleted(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(c1), dsl.PropDecl(p1)
                && dsl.IsMut(p1) && diff.ModuleVisible(p1))
            auto p2 = dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(c1));
            CHECK(RuleKind::CLASS_MEMBER_PROP_MUT_ATTR_DELETED, dsl.IsMut(p2), p1, p2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberDepreAnnoAddedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1),
            (dsl.Func(f1) || dsl.PropDecl(f1)) && diff.ModuleVisible(f1))
            LETIF(f2, dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1)), dsl.Func(f2) || dsl.PropDecl(f2))
            auto [v1DeprecatedAnno, v1Strict] = dsl.CheckDepreAnnotation(f1);
            auto [v2DeprecatedAnno, v2Strict] = dsl.CheckDepreAnnotation(f2);
            auto deprecatedAdded = !v1DeprecatedAnno && v2DeprecatedAnno && v2Strict;
            auto deprecatedChanged = v1DeprecatedAnno && v2DeprecatedAnno && (!v1Strict) && v2Strict;
            if (dsl.Func(f1)) {
                if (f1->TestAttr(Attribute::CONSTRUCTOR)) {
                    if (dsl.IsPublic(f1)) {
                        CHECK(RuleKind::CLASS_CONSTRUCTOR_DEPRE_ANNO_ADDED, !deprecatedAdded, f1, f2);
                        CHECK(RuleKind::CLASS_CONSTRUCTOR_DEPRE_ANNO_CHANGED, !deprecatedChanged, f1, f2);
                    }
                } else {
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_DEPRE_ANNO_ADDED, !deprecatedAdded, f1, f2);
                    CHECK(RuleKind::CLASS_INSTANCE_MEMBER_FUNC_DEPRE_ANNO_CHANGED, !deprecatedChanged, f1, f2);
                }
            }
            else if (dsl.PropDecl(f1)) {
                CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_DEPRE_ANNO_ADDED, !deprecatedAdded, f1, f2);
                CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_DEPRE_ANNO_CHANGED, !deprecatedChanged, f1, f2);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}
