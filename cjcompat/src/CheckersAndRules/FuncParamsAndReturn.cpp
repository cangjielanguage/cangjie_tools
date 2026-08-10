// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

bool CheckerImpl::FuncParametersChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    std::vector<Node*> checkedNode;
    BEGIN_FORALL(f1, diff.GetDeleted(), dsl.Func(f1) && diff.ModuleVisible(f1));
        BEGIN_FORALL(f2, diff.GetAdded(), dsl.Func(f2) && dsl.SameIdentifier(f1, f2) &&
            !IsCheckedAlready(checkedNode, f2));
            NodeInfo funcInfo{f1, f2};
            if (ChangeFuncParamOrder(dsl, logger, checker, checkerResult, funcInfo)) {
                checkedNode.emplace_back(f2);
                break;
            }
            // The parameter of member function has been changed.
            CheckFuncParams(dsl, logger, checker, checkerResult, funcInfo);
        END_FORALL()
    END_FORALL()
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.Func(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1));
        auto f2 = dsl.Corresponding(f1, diff.PotentiallyModified());
        if (!(dsl.Func(f2) && dsl.TopLevel(f2))) {
            continue;
        }
        NodeInfo funcInfo{f1, f2};
        if (ChangeFuncParamOrder(dsl, logger, checker, checkerResult, funcInfo)) {
            continue;
        }
        // The parameter of member function has been changed.
        CheckFuncParams(dsl, logger, checker, checkerResult, funcInfo);
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::FuncReturnType(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(f1, diff.GetDomPotentiallyModified(), dsl.Func(f1) && dsl.TopLevel(f1) && diff.ModuleVisible(f1))
        auto f2 = dsl.Corresponding(f1, diff.PotentiallyModified());
        auto t1 = dsl.ReturnType(f1);
        auto t2 = dsl.ReturnType(f2);
        // The return type A is changed to B.
        // API: When B is a subtype of A, the function is compatible. Conversely, Incompatible
        // ABI: When B is a subtype of A and both A and B are class or interface is compatible. Conversely, Incompatible
        if (!dsl.SameType(t1, t2)) {
            auto isSubtype = dsl.IsParentType(t1, t2);
            if (diff.ModuleVisible(f1) && diff.ModuleVisible(f2)) {
                CHECK(RuleKind::FUNC_RETURN_TYPE_SUBTYPE, isSubtype, f1, f2);
                CHECK(RuleKind::FUNC_RETURN_TYPE_CLASS_LIKE,
                    !isSubtype || (Dsl::IsClassLikeTy(t1) && Dsl::IsClassLikeTy(t2)), f1, f2);
            }
        }
    END_FORALL()
    return checkerResult;
}
