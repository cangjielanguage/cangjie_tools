// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

namespace {
static bool IsGenericParamOrderChanged(Ptr<Generic>& genericA, Ptr<Generic>& genericB)
{
    auto generic1Size = genericA ? genericA->typeParameters.size() : 0;
    auto generic2Size = genericB ? genericB->typeParameters.size() : 0;
    if (generic1Size != generic2Size) {
        return false;
    }
    bool isTypeChange = false;
    std::set<std::string> typesA;
    std::set<std::string> typesB;
    for (size_t i = 0; i < generic1Size; i++) {
        auto typeA = genericA->typeParameters[i].get();
        auto typeB = genericB->typeParameters[i].get();
        typesA.insert(typeA->identifier.GetRawText());
        typesB.insert(typeB->identifier.GetRawText());
        if (typeA->identifier.GetRawText() != typeB->identifier.GetRawText()) {
            isTypeChange = true;
        }
    }
    if (isTypeChange && typesA == typesB) {
        return true;
    }
    return false;
}

static bool checkUpperBounds(
    Ptr<GenericConstraint> gcA, Ptr<GenericConstraint> gcB, Dsl& dsl, bool& isParentType)
{
    if (gcA->upperBounds.size() > 1) {
        // func foo<T>(a: T) where T <: Eq1 & Eq2
        std::set<std::string> upperTypesA;
        std::set<std::string> upperTypesB;
        std::for_each(gcA->upperBounds.begin(), gcA->upperBounds.end(), [&](auto& upperType) {
            auto refType = dynamic_cast<RefType*>(upperType.get().get());
            upperTypesA.insert(refType->ref.identifier.GetRawText());
        });
        std::for_each(gcB->upperBounds.begin(), gcB->upperBounds.end(), [&](auto& upperType) {
            auto refType = dynamic_cast<RefType*>(upperType.get().get());
            upperTypesB.insert(refType->ref.identifier.GetRawText());
        });
        if (upperTypesA == upperTypesB) {
            return true;
        }
        return false;
    }
    auto upperTypeA = dynamic_cast<RefType*>(gcA->upperBounds[0].get().get());
    auto upperTypeB = dynamic_cast<RefType*>(gcB->upperBounds[0].get().get());
    if (upperTypeA->ref.identifier.GetRawText() == upperTypeB->ref.identifier.GetRawText()) {
        return true;
    }
    // When B is the parent type of A, the API is compatible. Conversely, it is not compatible.
    if (dsl.IsParentType(upperTypeB->GetTy(), upperTypeA->GetTy())) {
        isParentType = true;
        return false;
    }
    isParentType = false;
    return false;
}

static std::tuple<bool, bool> IsGenericConstraintCompatible(Ptr<Generic>& genericA, Ptr<Generic>& genericB, Dsl& dsl)
{
    auto generic1Size = genericA ? genericA->genericConstraints.size() : 0;
    auto generic2Size = genericB ? genericB->genericConstraints.size() : 0;
    if (generic1Size != generic2Size) {
        return std::make_tuple(true, false);
    }
    for (size_t i = 0; i < generic1Size; i++) {
        auto gcA = genericA->genericConstraints[i].get();
        for (size_t j = 0; j < generic2Size; j++) {
            auto gcB = genericB->genericConstraints[j].get();
            if (gcA->type->GetTy()->String() != gcB->type->GetTy()->String()) {
                continue;
            }
            if (gcA->upperBounds.size() != gcB->upperBounds.size()) {
                return std::make_tuple(true, false);
            }
            // one of genericConstraints has been changed.
            bool isParentType = true;
            if (!checkUpperBounds(gcA, gcB, dsl, isParentType)) {
                return std::make_tuple(false, !isParentType);
            }
        }
    }
    return std::make_tuple(true, false);
}

static void CheckFuncGenericParams(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo)
{
    auto f1 = funcInfo.n1;
    auto f2 = funcInfo.n2;
    auto f1GenericParams = dsl.GetNumGenericParams(f1);
    auto f2GenericParams = dsl.GetNumGenericParams(f2);
    CHECK(RuleKind::FUNC_GENERIC_PARAM_ADDED, f1GenericParams >= f2GenericParams, f1, f2);
    CHECK(RuleKind::FUNC_GENERIC_PARAM_DELETED, f1GenericParams <= f2GenericParams, f1, f2);
    auto g1 = dsl.GetGeneric(f1);
    auto g2 = dsl.GetGeneric(f2);
    CHECK(RuleKind::FUNC_GENERIC_PARAM_CHANGED, !IsGenericParamOrderChanged(g1, g2), f1, f2);
}

static std::tuple<size_t, size_t> GetNumGenericConstraints(Dsl& dsl, const Node* n1, const Node* n2)
{
    auto gc1 = dsl.GetNumGenericConstraints(n1);
    auto gc2 = dsl.GetNumGenericConstraints(n2);
    if (gc1 == gc2) {
        gc1 = dsl.GetNumGenericUpperBounds(n1);
        gc2 = dsl.GetNumGenericUpperBounds(n2);
    }
    return std::make_tuple(gc1, gc2);
}

static void CheckFuncGenericConstraints(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo)
{
    auto f1 = funcInfo.n1;
    auto f2 = funcInfo.n2;
    auto [f1GenericConstraints, f2GenericConstraints] = GetNumGenericConstraints(dsl, f1, f2);
    CHECK(RuleKind::FUNC_GENERIC_CONSTRAINT_ADDED, f1GenericConstraints >= f2GenericConstraints, f1, f2);
    CHECK(RuleKind::FUNC_GENERIC_CONSTRAINT_DELETED, f1GenericConstraints <= f2GenericConstraints, f1, f2);
    auto g1 = dsl.GetGeneric(f1);
    auto g2 = dsl.GetGeneric(f2);
    auto [isCompatible, isNotParentType] = IsGenericConstraintCompatible(g1, g2, dsl);
    if (isNotParentType) {
        CHECK(RuleKind::FUNC_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, false, f1, f2);
    } else {
        CHECK(RuleKind::FUNC_GENERIC_CONSTRAINT_CHANGED, isCompatible, f1, f2);
    }
}

static void CheckFuncGeneric(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo)
{
    CheckFuncGenericParams(dsl, logger, checker, checkerResult, funcInfo);
    CheckFuncGenericConstraints(dsl, logger, checker, checkerResult, funcInfo);
}

static void CheckStructGeneric(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& structInfo)
{
    auto s1 = structInfo.n1;
    auto s2 = structInfo.n2;
    auto s1GenericParams = dsl.GetNumGenericParams(s1);
    auto s2GenericParams = dsl.GetNumGenericParams(s2);
    CHECK(RuleKind::STRUCT_GENERIC_PARAM_ADDED, s1GenericParams >= s2GenericParams, s1, s2);
    CHECK(RuleKind::STRUCT_GENERIC_PARAM_DELETED, s1GenericParams <= s2GenericParams, s1, s2);
    auto g1 = dsl.GetGeneric(s1);
    auto g2 = dsl.GetGeneric(s2);
    CHECK(RuleKind::STRUCT_GENERIC_PARAM_CHANGED, !IsGenericParamOrderChanged(g1, g2), s1, s2);
    auto [s1GenericConstraints, s2GenericConstraints] = GetNumGenericConstraints(dsl, s1, s2);
    CHECK(RuleKind::STRUCT_GENERIC_CONSTRAINT_ADDED, s1GenericConstraints >= s2GenericConstraints, s1, s2);
    CHECK(RuleKind::STRUCT_GENERIC_CONSTRAINT_DELETED, s1GenericConstraints <= s2GenericConstraints, s1, s2);
    auto [isCompatible, isNotParentType] = IsGenericConstraintCompatible(g1, g2, dsl);
    if (isNotParentType) {
        CHECK(RuleKind::STRUCT_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, false, s1, s2);
    } else {
        CHECK(RuleKind::STRUCT_GENERIC_CONSTRAINT_CHANGED, isCompatible, s1, s2);
    }
}

static void CheckEnumGeneric(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& enumInfo)
{
    auto e1 = enumInfo.n1;
    auto e2 = enumInfo.n2;
    auto e1GenericParams = dsl.GetNumGenericParams(e1);
    auto e2GenericParams = dsl.GetNumGenericParams(e2);
    CHECK(RuleKind::ENUM_GENERIC_PARAM_ADDED, e1GenericParams >= e2GenericParams, e1, e2);
    CHECK(RuleKind::ENUM_GENERIC_PARAM_DELETED, e1GenericParams <= e2GenericParams, e1, e2);
    auto g1 = dsl.GetGeneric(e1);
    auto g2 = dsl.GetGeneric(e2);
    CHECK(RuleKind::ENUM_GENERIC_PARAM_CHANGED, !IsGenericParamOrderChanged(g1, g2), e1, e2);
    auto [e1GenericConstraints, e2GenericConstraints] = GetNumGenericConstraints(dsl, e1, e2);
    CHECK(RuleKind::ENUM_GENERIC_CONSTRAINT_ADDED, e1GenericConstraints >= e2GenericConstraints, e1, e2);
    CHECK(RuleKind::ENUM_GENERIC_CONSTRAINT_DELETED, e1GenericConstraints <= e2GenericConstraints, e1, e2);
    auto [isCompatible, isNotParentType] = IsGenericConstraintCompatible(g1, g2, dsl);
    if (isNotParentType) {
        CHECK(RuleKind::ENUM_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, false, e1, e2);
    } else {
        CHECK(RuleKind::ENUM_GENERIC_CONSTRAINT_CHANGED, isCompatible, e1, e2);
    }
}

static void CheckClassGeneric(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& classInfo)
{
    auto c1 = classInfo.n1;
    auto c2 = classInfo.n2;
    auto c1GenericParams = dsl.GetNumGenericParams(c1);
    auto c2GenericParams = dsl.GetNumGenericParams(c2);
    CHECK(RuleKind::CLASS_GENERIC_PARAM_ADDED, c1GenericParams >= c2GenericParams, c1, c2);
    CHECK(RuleKind::CLASS_GENERIC_PARAM_DELETED, c1GenericParams <= c2GenericParams, c1, c2);
    auto g1 = dsl.GetGeneric(c1);
    auto g2 = dsl.GetGeneric(c2);
    CHECK(RuleKind::CLASS_GENERIC_PARAM_CHANGED, !IsGenericParamOrderChanged(g1, g2), c1, c2);
    auto [c1GenericConstraints, c2GenericConstraints] = GetNumGenericConstraints(dsl, c1, c2);
    CHECK(RuleKind::CLASS_GENERIC_CONSTRAINT_ADDED, c1GenericConstraints >= c2GenericConstraints, c1, c2);
    CHECK(RuleKind::CLASS_GENERIC_CONSTRAINT_DELETED, c1GenericConstraints <= c2GenericConstraints, c1, c2);
    auto [isCompatible, isNotParentType] = IsGenericConstraintCompatible(g1, g2, dsl);
    if (isNotParentType) {
        CHECK(RuleKind::CLASS_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, false, c1, c2);
    } else {
        CHECK(RuleKind::CLASS_GENERIC_CONSTRAINT_CHANGED, isCompatible, c1, c2);
    }
}

static void CheckInterfaceGeneric(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& interfaceInfo)
{
    auto i1 = interfaceInfo.n1;
    auto i2 = interfaceInfo.n2;
    auto i1GenericParams = dsl.GetNumGenericParams(i1);
    auto i2GenericParams = dsl.GetNumGenericParams(i2);
    CHECK(RuleKind::INTERFACE_GENERIC_PARAM_ADDED, i1GenericParams >= i2GenericParams, i1, i2);
    CHECK(RuleKind::INTERFACE_GENERIC_PARAM_DELETED, i1GenericParams <= i2GenericParams, i1, i2);
    auto g1 = dsl.GetGeneric(i1);
    auto g2 = dsl.GetGeneric(i2);
    CHECK(RuleKind::INTERFACE_GENERIC_PARAM_CHANGED, !IsGenericParamOrderChanged(g1, g2), i1, i2);
    auto [i1GenericConstraints, i2GenericConstraints] = GetNumGenericConstraints(dsl, i1, i2);
    CHECK(RuleKind::INTERFACE_GENERIC_CONSTRAINT_ADDED, i1GenericConstraints >= i2GenericConstraints, i1, i2);
    CHECK(RuleKind::INTERFACE_GENERIC_CONSTRAINT_DELETED, i1GenericConstraints <= i2GenericConstraints, i1, i2);
    auto [isCompatible, isNotParentType] = IsGenericConstraintCompatible(g1, g2, dsl);
    if (isNotParentType) {
        CHECK(RuleKind::INTERFACE_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, false, i1, i2);
    } else {
        CHECK(RuleKind::INTERFACE_GENERIC_CONSTRAINT_CHANGED, isCompatible, i1, i2);
    }
}

static void CheckExtendGeneric(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& extendInfo)
{
    auto e1 = extendInfo.n1;
    auto e2 = extendInfo.n2;
    auto e1GenericParams = dsl.GetNumGenericParams(e1);
    auto e2GenericParams = dsl.GetNumGenericParams(e2);
    CHECK(RuleKind::EXTEND_GENERIC_PARAM_ADDED, e1GenericParams >= e2GenericParams, e1, e2);
    CHECK(RuleKind::EXTEND_GENERIC_PARAM_DELETED, e1GenericParams <= e2GenericParams, e1, e2);
    auto g1 = dsl.GetGeneric(e1);
    auto g2 = dsl.GetGeneric(e2);
    CHECK(RuleKind::EXTEND_GENERIC_PARAM_CHANGED, !IsGenericParamOrderChanged(g1, g2), e1, e2);
    auto [e1GenericConstraints, e2GenericConstraints] = GetNumGenericConstraints(dsl, e1, e2);
    CHECK(RuleKind::EXTEND_GENERIC_CONSTRAINT_ADDED, e1GenericConstraints >= e2GenericConstraints, e1, e2);
    CHECK(RuleKind::EXTEND_GENERIC_CONSTRAINT_DELETED, e1GenericConstraints <= e2GenericConstraints, e1, e2);
    auto [isCompatible, isNotParentType] = IsGenericConstraintCompatible(g1, g2, dsl);
    if (isNotParentType) {
        CHECK(RuleKind::EXTEND_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, false, e1, e2);
    } else {
        CHECK(RuleKind::EXTEND_GENERIC_CONSTRAINT_CHANGED, isCompatible, e1, e2);
    }
}
}

bool CheckerImpl::FuncGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDeleted(), dsl.Func(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1));
        BEGIN_FORALL(f2, diff.GetAdded(),
            dsl.Func(f2) && dsl.TopLevel(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2));
            NodeInfo funcInfo{f1, f2};
            CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
        END_FORALL()
    END_FORALL()
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.Func(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1));
        auto f2 = dsl.Corresponding(f1, diff.PotentiallyModified());
        NodeInfo funcInfo{f1, f2, &diff};
        CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDeleted(),
        dsl.StructDecl(s1) && dsl.TopLevel(s1) && diff.ModuleVisible(s1));
        BEGIN_FORALL(s2, diff.GetAdded(),
            dsl.StructDecl(s2) && dsl.TopLevel(s2) && diff.ModuleVisible(s2) && dsl.SameIdentifier(s1, s2));
            NodeInfo structInfo{s1, s2};
            CheckStructGeneric(dsl, logger, checker, checkerResult, structInfo);
        END_FORALL()
    END_FORALL()
    BEGIN_FORALL(s1, diff.GetDomPotentiallyModified(),
        dsl.StructDecl(s1) && dsl.TopLevel(s1) && diff.ModuleVisible(s1));
        auto s2 = dsl.Corresponding(s1, diff.PotentiallyModified());
        NodeInfo structInfo{s1, s2};
        CheckStructGeneric(dsl, logger, checker, checkerResult, structInfo);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDeleted(), dsl.EnumDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        BEGIN_FORALL(e2, diff.GetAdded(),
            dsl.EnumDecl(e2) && dsl.TopLevel(e2) && diff.ModuleVisible(e2) && dsl.SameIdentifier(e1, e2));
            NodeInfo enumInfo{e1, e2};
            CheckEnumGeneric(dsl, logger, checker, checkerResult, enumInfo);
        END_FORALL()
    END_FORALL()
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        auto e2 = dsl.Corresponding(e1, diff.PotentiallyModified());
        NodeInfo enumInfo{e1, e2};
        CheckEnumGeneric(dsl, logger, checker, checkerResult, enumInfo);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDeleted(), dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1));
        BEGIN_FORALL(c2, diff.GetAdded(),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2) && dsl.SameIdentifier(c1, c2));
            NodeInfo classInfo{c1, c2};
            CheckClassGeneric(dsl, logger, checker, checkerResult, classInfo);
        END_FORALL()
    END_FORALL()
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(), dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1));
        auto c2 = dsl.Corresponding(c1, diff.PotentiallyModified());
        NodeInfo classInfo{c1, c2};
        CheckClassGeneric(dsl, logger, checker, checkerResult, classInfo);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(i1, diff.GetDeleted(),
        dsl.InterfaceDecl(i1) && dsl.TopLevel(i1) && diff.ModuleVisible(i1));
        BEGIN_FORALL(i2, diff.GetAdded(),
            dsl.InterfaceDecl(i2) && dsl.TopLevel(i2) && diff.ModuleVisible(i2) && dsl.SameIdentifier(i1, i2));
            NodeInfo interfaceInfo{i1, i2};
            CheckInterfaceGeneric(dsl, logger, checker, checkerResult, interfaceInfo);
        END_FORALL()
    END_FORALL()
    BEGIN_FORALL(i1, diff.GetDomPotentiallyModified(),
        dsl.InterfaceDecl(i1) && dsl.TopLevel(i1) && diff.ModuleVisible(i1));
        auto i2 = dsl.Corresponding(i1, diff.PotentiallyModified());
        NodeInfo interfaceInfo{i1, i2};
        CheckInterfaceGeneric(dsl, logger, checker, checkerResult, interfaceInfo);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ExtendGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDeleted(),
        dsl.ExtendDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        BEGIN_FORALL(e2, diff.GetAdded(),
            dsl.ExtendDecl(e2) && dsl.TopLevel(e2) && diff.ModuleVisible(e2) && diff.SameExtend(e1, e2));
            NodeInfo extendInfo{e1, e2};
            CheckExtendGeneric(dsl, logger, checker, checkerResult, extendInfo);
        END_FORALL()
    END_FORALL()
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(),
        dsl.ExtendDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        auto e2 = dsl.Corresponding(e1, diff.PotentiallyModified());
        NodeInfo extendInfo{e1, e2};
        CheckExtendGeneric(dsl, logger, checker, checkerResult, extendInfo);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructMemberFuncGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDomPotentiallyModified(),
        dsl.StructDecl(s1) && dsl.TopLevel(s1) && diff.ModuleVisible(s1));
        LETIF(s2, dsl.Corresponding(s1, diff.PotentiallyModified()), dsl.TopLevel(s2) && diff.ModuleVisible(s2))
        BEGIN_FORALL(f1, diff.GetMemberDeleted(s1), dsl.Func(f1) && diff.ModuleVisible(f1));
            BEGIN_FORALL(f2, diff.GetMemberAdded(s1),
                dsl.Func(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2));
                NodeInfo funcInfo{f1, f2};
                CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
            END_FORALL()
        END_FORALL()
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(s1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(s1));
            NodeInfo funcInfo{f1, f2};
            CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumMemberFuncGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(),
        dsl.EnumDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        LETIF(e2, dsl.Corresponding(e1, diff.PotentiallyModified()), dsl.TopLevel(e2) && diff.ModuleVisible(e2))
        BEGIN_FORALL(f1, diff.GetMemberDeleted(e1), dsl.Func(f1) && diff.ModuleVisible(f1));
            BEGIN_FORALL(f2, diff.GetMemberAdded(e1),
                dsl.Func(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2));
                NodeInfo funcInfo{f1, f2};
                CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
            END_FORALL()
        END_FORALL()
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(e1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(e1));
            NodeInfo funcInfo{f1, f2};
            CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassMemberFuncGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(f1, diff.GetMemberDeleted(c1), dsl.Func(f1) && diff.ModuleVisible(f1))
            BEGIN_FORALL(f2, diff.GetMemberAdded(c1),
                dsl.Func(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2))
                NodeInfo funcInfo{f1, f2};
                CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
            END_FORALL()
        END_FORALL()
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1), dsl.Func(f1) && diff.ModuleVisible(f1))
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1));
            NodeInfo funcInfo{f1, f2};
            CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ExtendMemberFuncGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(),
        dsl.ExtendDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        LETIF(e2, dsl.Corresponding(e1, diff.PotentiallyModified()), dsl.TopLevel(e2) && diff.ModuleVisible(e2))
        BEGIN_FORALL(f1, diff.GetMemberDeleted(e1), dsl.Func(f1) && diff.ModuleVisible(f1));
            BEGIN_FORALL(f2, diff.GetMemberAdded(e1),
                dsl.Func(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2));
                NodeInfo funcInfo{f1, f2};
                CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
            END_FORALL()
        END_FORALL()
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(e1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(e1));
            NodeInfo funcInfo{f1, f2};
            CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceMemberFuncGenericAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(i1, diff.GetDomPotentiallyModified(),
        dsl.InterfaceDecl(i1) && dsl.TopLevel(i1) && diff.ModuleVisible(i1));
        LETIF(i2, dsl.Corresponding(i1, diff.PotentiallyModified()), dsl.TopLevel(i2) && diff.ModuleVisible(i2))
        BEGIN_FORALL(f1, diff.GetMemberDeleted(i1), dsl.Func(f1) && diff.ModuleVisible(f1));
            BEGIN_FORALL(f2, diff.GetMemberAdded(i1),
                dsl.Func(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2));
                NodeInfo funcInfo{f1, f2};
                CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
            END_FORALL()
        END_FORALL()
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(i1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(i1));
            NodeInfo funcInfo{f1, f2};
            CheckFuncGeneric(dsl, logger, checker, checkerResult, funcInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}
