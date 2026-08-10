// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/CheckersAndRules/Checker.h"

namespace {
static bool IsAttributeUnsafeOrMutOrStaticAdded(Node* f1, Node* f2)
{
    return (!f1->TestAttr(Attribute::UNSAFE) && f2->TestAttr(Attribute::UNSAFE)) ||
        (!f1->TestAttr(Attribute::MUT) && f2->TestAttr(Attribute::MUT)) ||
        (!f1->TestAttr(Attribute::STATIC) && f2->TestAttr(Attribute::STATIC));
}

static bool IsAttributeMutDeleted(Node* f1, Node* f2)
{
    return f1->TestAttr(Attribute::MUT) && !f2->TestAttr(Attribute::MUT);
}

static bool IsAttributeMutAdded(Node* f1, Node* f2)
{
    return !f1->TestAttr(Attribute::MUT) && f2->TestAttr(Attribute::MUT);
}

static bool IsAttributeUnsafeAdded(Node* f1, Node* f2)
{
    return !f1->TestAttr(Attribute::UNSAFE) && f2->TestAttr(Attribute::UNSAFE);
}

static bool isAttributeConstOrStaticDeleted(Dsl& dsl, Node* f1, Node* f2)
{
    return (dsl.IsConst(f1) && !dsl.IsConst(f2)) ||
        (f1->TestAttr(Attribute::STATIC) && !f2->TestAttr(Attribute::STATIC));
}

static bool isAttributeConstOrStaticAdded(Dsl& dsl, Node* f1, Node* f2)
{
    return (!dsl.IsConst(f1) && dsl.IsConst(f2)) ||
        (!f1->TestAttr(Attribute::STATIC) && f2->TestAttr(Attribute::STATIC));
}

static bool IsClassExtendedType(Node* n)
{
    auto ed = dynamic_cast<const ExtendDecl*>(n);
    return ed && ed->extendedType && ed->extendedType->GetTy() && ed->extendedType->GetTy()->IsClass();
}

static bool IsStructExtendedType(Node* n)
{
    auto ed = dynamic_cast<const ExtendDecl*>(n);
    return ed && ed->extendedType && ed->extendedType->GetTy() && ed->extendedType->GetTy()->IsStruct();
}

static bool IsDepreAnnotationChanged(Dsl& dsl, Node* f1, Node* f2)
{
    auto [f1DeprecatedAnno, f1Strict] = dsl.CheckDepreAnnotation(f1);
    auto [f2DeprecatedAnno, f2Strict] = dsl.CheckDepreAnnotation(f2);
    // The annotation(Deprecated) of member function has been added or changed to strict.
    auto funcAnnoChanged = (f1DeprecatedAnno && !f1Strict && f2DeprecatedAnno && f2Strict) ||
        (!f1DeprecatedAnno && f2DeprecatedAnno && f2Strict);
    return funcAnnoChanged;
}

static bool IsFrozenAnnotationDeleted(Dsl& dsl, Node* f1, Node* f2)
{
    return dsl.IsFrozen(f1) && !dsl.IsFrozen(f2);
}

static Ptr<Decl> GetOuterDecl(Node* n)
{
    auto d = dynamic_cast<const Decl*>(n);
    if (d == nullptr) {
        return nullptr;
    }
    return d->outerDecl;
}

static bool AddMemberParamDefaultValue(Dsl& dsl, Node* f, Node* p1, Node* p2)
{
    auto pDecl = GetOuterDecl(f);
    if (!pDecl) {
        return false;
    }
    if (!dsl.HasConstOrFrozenInit(pDecl)) {
        return false;
    }
    if (dsl.IsMemberParam(p1) && dsl.IsMemberParam(p2) &&
        !dsl.HasDefaultValue(p1) && dsl.HasDefaultValue(p2)) {
        return true;
    }
    return false;
}

static bool IsClassOpenInstanceFunc(Dsl& dsl, Node* fd)
{
    auto pDecl = GetOuterDecl(fd);
    if (!pDecl || pDecl->astKind != ASTKind::CLASS_DECL) {
        return false;
    }
    if (fd->TestAttr(Attribute::CONSTRUCTOR) || fd->TestAttr(Attribute::STATIC)) {
        return false;
    }
    if (fd->TestAttr(Attribute::OPEN)) {
        return true;
    }
    return false;
}

static void CheckFuncReturnType(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo)
{
    auto f1 = funcInfo.n1;
    auto f2 = funcInfo.n2;
    if (f1->TestAnyAttr(Attribute::CONSTRUCTOR) ||
        f1->TestAnyAttr(Attribute::ENUM_CONSTRUCTOR)) {
        return;
    }
    auto t1 = dsl.ReturnType(f1);
    auto t2 = dsl.ReturnType(f2);
    if (dsl.SameType(t1, t2)) {
        return;
    }
    // The return type of function is changed.
    if (IsClassOpenInstanceFunc(dsl, f1)) {
        CHECK(RuleKind::FUNC_RETURN_TYPE_OPEN, false, f1, f2);
        return;
    }
    auto isParentType = dsl.IsParentType(t1, t2);
    // API: compatible when t2 is a subtype of t1; conversely, incompatible.
    CHECK(RuleKind::FUNC_RETURN_TYPE_SUBTYPE, isParentType, f1, f2);
    // ABI: compatible when t2 is a subtype of t1 and both t1 and t2 are class or interface;
    //      conversely, incompatible.
    auto abiCompatible = !isParentType // No need to check further if !isParentType. Already Reported above!
        || ((t1->IsClass() || t1->IsInterface()) && (t2->IsClass() || t2->IsInterface()));
    CHECK(RuleKind::FUNC_RETURN_TYPE_CLASS_LIKE, abiCompatible, f1, f2);
}

static void CheckPropBodyChanged(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& propInfo)
{
    auto p1 = propInfo.n1;
    auto p2 = propInfo.n2;
    if (!dsl.IsFrozen(p1) || !dsl.IsFrozen(p2)) {
        return;
    }
    auto decl1 = dynamic_cast<const PropDecl*>(p1);
    auto decl2 = dynamic_cast<const PropDecl*>(p2);
    if (!decl1 || !decl2) {
        return;
    }
    if (!decl1->getters.empty() && !decl2->getters.empty()) {
        auto fget1 = decl1->getters[0].get();
        auto fget2 = decl2->getters[0].get();
        auto sameGetter = dsl.BodyHashOf(fget1) == dsl.BodyHashOf(fget2);
        CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, sameGetter, fget1, fget2);
    }
    if (!decl1->setters.empty() && !decl2->setters.empty()) {
        auto fset1 = decl1->setters[0].get();
        auto fset2 = decl2->setters[0].get();
        auto sameSetter = dsl.BodyHashOf(fset1) == dsl.BodyHashOf(fset2);
        CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, sameSetter, fset1, fset2);
    }
}

static bool CheckConstructorParam(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo)
{
    auto f1 = funcInfo.n1;
    auto f2 = funcInfo.n2;
    auto numParams = dsl.NumFuncParams(f1);
    auto numParams2 = dsl.NumFuncParams(f2);
    if (f1->TestAnyAttr(Attribute::CONSTRUCTOR) && numParams + 1 == numParams2) {
        bool sameParamType = true;
        for (auto i : dsl.Range(0, numParams)) {
            auto p1 = dsl.GetFuncParam(f1, i);
            auto p2 = dsl.GetFuncParam(f2, i);
            if (!p1 || !p2) {
                continue;
            }
            if (!dsl.SameType(p1->GetTy(), p2->GetTy()) || dsl.NameOfParam(p1) != dsl.NameOfParam(p2)) {
                sameParamType = false;
                break;
            }
        }
        auto p = dsl.GetFuncParam(f2, numParams2 - 1);
        // Compatible if the added parameter is at the end of function parameters and has a default value.
        // Conversely, incompatible.
        if (sameParamType && dsl.IsNamedParam(p) && dsl.HasDefaultValue(p)) {
            CHECK(RuleKind::FUNC_PARAM_ADDED_ABI, false, f1, f2);
            return true;
        }
    }
    return false;
}

static std::tuple<bool, bool> CheckPrivateMemberParam(Node* f1)
{
    bool hasMember = false;
    bool hasPrivateMember = false;
    auto numParams = Dsl::NumFuncParams(f1);
    for (auto i : Dsl::Range(0, numParams)) {
        auto p = Dsl::GetFuncParam(f1, i);
        if (!Dsl::IsMemberParam(p)) {
            continue;
        }
        // The constructor has a member parameter.
        hasMember = true;
        if (!p->TestAttr(Attribute::PRIVATE)) {
            // The constructor has a non-private member parameter.
            return std::make_tuple(true, false);
        }
        hasPrivateMember = true;
    }
    return std::make_tuple(hasMember, hasPrivateMember);
}

static bool HasMemberParam(Node* f)
{
    auto numParams = Dsl::NumFuncParams(f);
    for (auto i : Dsl::Range(0, numParams)) {
        auto p = Dsl::GetFuncParam(f, i);
        if (Dsl::IsMemberParam(p)) {
            return true;
        }
    }
    return false;
}

static bool HasSameMemberParam(Node* f1, Node* v1)
{
    auto numParams = Dsl::NumFuncParams(f1);
    for (auto i : Dsl::Range(0, numParams)) {
        auto p = Dsl::GetFuncParam(f1, i);
        if (Dsl::IsMemberParam(p) && Dsl::SameIdentifier(p, v1)) {
            return true;
        }
    }
    return false;
}

static bool HasSameConstructor(Node* f, std::set<Node*>& addOrDelNodes, std::set<Node*>& modifyNodes)
{
    BEGIN_FORALL(f2, addOrDelNodes, Dsl::Func(f2) && f2->TestAttr(Attribute::CONSTRUCTOR));
        return true;
    END_FORALL()
    // The private primary function is not in the cjo.
    // Use the member parameter to check whether the function exists.
    BEGIN_FORALL(v, modifyNodes, Dsl::VarLetOrConst(v) && Dsl::IsMemberParam(v));
        if (HasSameMemberParam(f, v)) {
            return true;
        }
    END_FORALL()
    return false;
}

static void CheckConstructorChanged(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& nodeInfo)
{
    auto n = nodeInfo.n1;
    auto diff = *nodeInfo.diff;
    auto addNodes = diff.GetMemberAdded(n);
    auto delNodes = diff.GetMemberDeleted(n);
    auto modifyNodes = diff.GetDomPotentiallyMemberModified(n);
    BEGIN_FORALL(f1, delNodes, dsl.Func(f1) && f1->TestAttr(Attribute::CONSTRUCTOR) && !dsl.IsPublic(f1));
        if (HasSameConstructor(f1, addNodes, modifyNodes)) {
            continue;
        }
        // The primary constructor with member variable parameter has been deleted.
        auto [hasMember, isPrivate] = CheckPrivateMemberParam(f1);
        if (hasMember) {
            if (isPrivate) {
                CHECK(RuleKind::PRIMARY_CONSTRUCTOR_NON_PUBLIC_DELETED2, false, f1);
            } else {
                CHECK(RuleKind::PRIMARY_CONSTRUCTOR_NON_PUBLIC_DELETED, false, f1);
            }
        }
    END_FORALL()
    BEGIN_FORALL(f1, addNodes, dsl.Func(f1) && f1->TestAttr(Attribute::CONSTRUCTOR));
        if (HasSameConstructor(f1, delNodes, modifyNodes)) {
            continue;
        }
        // The primary constructor with member variable parameter has been added.
        if (HasMemberParam(f1)) {
            CHECK(RuleKind::PRIMARY_CONSTRUCTOR_ADDED, false, f1);
        }
    END_FORALL()
}

static void CheckMemberFuncParams(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& nodeInfo)
{
    auto n = nodeInfo.n1;
    auto diff = *nodeInfo.diff;
    std::vector<Node*> checkedNode;
    BEGIN_FORALL(f1, diff.GetMemberDeleted(n), dsl.Func(f1) && diff.ModuleVisible(f1));
        auto hasSameFunc = false;
        BEGIN_FORALL(f2, diff.GetMemberAdded(n), dsl.Func(f2) && diff.ModuleVisible(f2) &&
            !CheckerImpl::IsCheckedAlready(checkedNode, f2) && dsl.SameIdentifier(f1, f2));
            NodeInfo funcInfo{f1, f2};
            if (CheckerImpl::ChangeFuncParamOrder(dsl, logger, checker, checkerResult, funcInfo) ||
                CheckConstructorParam(dsl, logger, checker, checkerResult, funcInfo)) {
                checkedNode.emplace_back(f2);
                hasSameFunc = true;
                break;
            }
        END_FORALL()
        if (hasSameFunc) {
            continue;
        }
        BEGIN_FORALL(f2, diff.GetMemberAdded(n), dsl.Func(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2));
            NodeInfo funcInfo{f1, f2};
            // The parameter of member function has been changed.
            CheckerImpl::CheckFuncParams(dsl, logger, checker, checkerResult, funcInfo);
            hasSameFunc = true;
        END_FORALL()
        // The member function has been deleted.
        if (dsl.IsPublic(f1)) {
            if (n->astKind == ASTKind::STRUCT_DECL) {
                auto ruleKind = f1->TestAnyAttr(Attribute::CONSTRUCTOR) ? RuleKind::STRUCT_INIT_FUNC_DELETED
                                                                        : RuleKind::STRUCT_MEMBER_FUNC_DELETED;
                CHECK(ruleKind, hasSameFunc, f1);
            } else if (n->astKind == ASTKind::ENUM_DECL) {
                CHECK(RuleKind::ENUM_MEMBER_FUNC_DELETED, hasSameFunc, f1);
            } else if (n->astKind == ASTKind::INTERFACE_DECL) {
                CHECK(RuleKind::INTERFACE_MEMBER_FUNC_DELETED, hasSameFunc, f1);
            }
        }
        if (n->astKind == ASTKind::CLASS_DECL) {
            if (dsl.IsPublic(f1) || dsl.IsProtected(f1)) {
                auto ruleKind = f1->TestAnyAttr(Attribute::CONSTRUCTOR) ?
                    RuleKind::CLASS_CONSTRUCTOR_DELETED : RuleKind::CLASS_INSTANCE_MEMBER_FUNCS_DELETED;
                CHECK(ruleKind, hasSameFunc, f1);
            }
        }
    END_FORALL()
    BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(n), dsl.Func(f1));
        auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(n));
        if (!(diff.ModuleVisible(f1) || diff.ModuleVisible(f2))) {
            continue;
        }
        NodeInfo funcInfo{f1, f2};
        if (CheckerImpl::ChangeFuncParamOrder(dsl, logger, checker, checkerResult, funcInfo)) {
            continue;
        }
        // The parameter of member function has been changed.
        CheckerImpl::CheckFuncParams(dsl, logger, checker, checkerResult, funcInfo);
    END_FORALL()
    CheckConstructorChanged(dsl, logger, checker, checkerResult, nodeInfo);
}

static void SetMemberDecls(std::vector<std::string>& dels, InterfaceDecl* id)
{
    for (auto& member : id->GetMemberDecls()) {
        if (member->astKind != ASTKind::FUNC_DECL && member->astKind != ASTKind::PROP_DECL) {
            continue;
        }
        dels.emplace_back(member->mangledName);
    }
}

static void ProccessMemberDecls(std::vector<std::string>& dels1, std::vector<std::string>& dels2)
{
    std::set<std::string> id2memberDeclsSet(dels2.begin(), dels2.end());
    dels1.erase(
        std::remove_if(
            dels1.begin(),
            dels1.end(),
            [&id2memberDeclsSet](const std::string& str) {
                return id2memberDeclsSet.find(str) == id2memberDeclsSet.end();
            }),
        dels1.end()
    );
    std::set<std::string> id1memberDeclsSet(dels1.begin(), dels1.end());
    dels2.erase(
        std::remove_if(
            dels2.begin(),
            dels2.end(),
            [&id1memberDeclsSet](const std::string& str) {
                return id1memberDeclsSet.find(str) == id1memberDeclsSet.end();
            }),
        dels2.end()
    );
}

static bool CheckInterfaceMemberFuncOrder(
    std::vector<std::string>& id1memberDecls, std::vector<std::string>& id2memberDecls)
{
    ProccessMemberDecls(id1memberDecls, id2memberDecls);
    for (size_t i = 0; i < id1memberDecls.size(); ++i) {
        auto mn1 = id1memberDecls[i];
        for (size_t j = 0; j < id2memberDecls.size(); ++j) {
            auto mn2 = id2memberDecls[j];
            if (mn1 != mn2) {
                continue;
            }
            if (i != j) {
                return true;
            }
        }
    }
    return false;
}

static void CheckInterfaceMemberFuncAdded(
    std::vector<std::string>& id1memberDecls, std::vector<std::string>& id2memberDecls,
    std::vector<Node*> &diagNode, std::vector<Node*> &lastNode, NodeInfo& nodeInfo)
{
    auto diff = *nodeInfo.diff;
    std::vector<Node*> checkedNode;
    BEGIN_FORALL(f1, diff.GetMemberAdded(nodeInfo.n1), Dsl::Func(f1));
        auto hasSameFunc = false;
        BEGIN_FORALL(f2, diff.GetMemberDeleted(nodeInfo.n1), Dsl::Func(f2)
                && Dsl::SameIdentifier(f1, f2) && !CheckerImpl::IsCheckedAlready(checkedNode, f2));
            hasSameFunc = true;
        END_FORALL()
        bool afterLastFunc = false;
        std::string lastfunc = "";
        if (id1memberDecls.empty()) {
            afterLastFunc = true;
        } else {
            lastfunc = id1memberDecls.back();
        }
        auto fd = dynamic_cast<FuncDecl*>(f1);
        for (auto& memberFunc : id2memberDecls) {
            if (memberFunc == lastfunc) {
                afterLastFunc = true;
                continue;
            }
            if (fd->mangledName == memberFunc && !hasSameFunc) {
                if (!afterLastFunc) {
                    diagNode.emplace_back(f1);
                } else {
                    lastNode.emplace_back(f1);
                }
            }
        }
    END_FORALL()
}

static void CheckInterfaceMemberPropAdded(
    std::vector<std::string>& id1memberDecls, std::vector<std::string>& id2memberDecls,
    std::vector<Node*> &diagNode, std::vector<Node*> &lastNode, NodeInfo& nodeInfo)
{
    auto diff = *nodeInfo.diff;
    std::vector<Node*> checkedNode;
    BEGIN_FORALL(p1, diff.GetMemberAdded(nodeInfo.n1), Dsl::PropDecl(p1) && diff.ModuleVisible(p1));
        auto hasSameFunc = false;
        BEGIN_FORALL(p2, diff.GetMemberDeleted(nodeInfo.n1), Dsl::PropDecl(p2) && diff.ModuleVisible(p2)
                && Dsl::SameIdentifier(p1, p2) && !CheckerImpl::IsCheckedAlready(checkedNode, p2));
            hasSameFunc = true;
        END_FORALL()
        bool afterLastFunc = false;
        std::string lastfunc = "";
        if (id1memberDecls.empty()) {
            afterLastFunc = true;
        } else {
            lastfunc = id1memberDecls.back();
        }
        auto fd = dynamic_cast<PropDecl*>(p1);
        for (auto& memberFunc : id2memberDecls) {
            if (memberFunc == lastfunc) {
                afterLastFunc = true;
                continue;
            }
            if (fd->mangledName == memberFunc && !hasSameFunc) {
                if (!afterLastFunc) {
                    diagNode.emplace_back(p1);
                } else {
                    lastNode.emplace_back(p1);
                }
            }
        }
    END_FORALL()
}

static void CheckInterfaceMemberFuncOrderAndAdded(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& nodeInfo)
{
    auto id1 = dynamic_cast<InterfaceDecl*>(nodeInfo.n1);
    auto id2 = dynamic_cast<InterfaceDecl*>(nodeInfo.n2);
    std::vector<std::string> id1memberDecls;
    std::vector<std::string> id2memberDecls;
    SetMemberDecls(id1memberDecls, id1);
    SetMemberDecls(id2memberDecls, id2);
    std::vector<Node*> diagNode;
    std::vector<Node*> lastNode;
    CheckInterfaceMemberFuncAdded(id1memberDecls, id2memberDecls, diagNode, lastNode, nodeInfo);
    CheckInterfaceMemberPropAdded(id1memberDecls, id2memberDecls, diagNode, lastNode, nodeInfo);
    for (auto& node : diagNode) {
        if (node->astKind == ASTKind::FUNC_DECL) {
            if (!node->TestAttr(Attribute::DEFAULT)) {
                CHECK(RuleKind::INTERFACE_MEMBER_FUNC_WITHOUT_DEFAULT_IMP_ADDED, false, node);
                continue;
            }
            if (dsl.IsStatic(node)) {
                CHECK(RuleKind::INTERFACE_STATIC_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(nodeInfo.n1, node), node);
            } else {
                CHECK(RuleKind::INTERFACE_INSTANCE_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(nodeInfo.n1, node), node);
            }
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_ADDED, dsl.HasUnimplementedDecl(nodeInfo.n1, node), node);
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_WITHOUT_DEFAULT_IMP_ADDED2,
                !dsl.HasUnimplementedDecl(nodeInfo.n1, node), node);
        } else if (node->astKind == ASTKind::PROP_DECL) {
            if (!node->TestAttr(Attribute::DEFAULT)) {
                CHECK(RuleKind::INTERFACE_MEMBER_PROP_WITHOUT_DEFAULT_IMP_ADDED, false, node);
                continue;
            }
            if (dsl.IsStatic(node)) {
                CHECK(RuleKind::INTERFACE_STATIC_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(nodeInfo.n1, node), node);
            } else {
                CHECK(RuleKind::INTERFACE_INSTANCE_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(nodeInfo.n1, node), node);
            }
            CHECK(RuleKind::INTERFACE_MEMBER_PROP_ADDED, dsl.HasUnimplementedDecl(nodeInfo.n1, node), node);
            CHECK(RuleKind::INTERFACE_MEMBER_PROP_WITHOUT_DEFAULT_IMP_ADDED2,
                !dsl.HasUnimplementedDecl(nodeInfo.n1, node), node);
        }
    }
    for (auto& node : lastNode) {
        if (node->astKind == ASTKind::FUNC_DECL) {
            if (dsl.IsStatic(node)) {
                CHECK(RuleKind::INTERFACE_STATIC_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(nodeInfo.n1, node), node);
            } else {
                CHECK(RuleKind::INTERFACE_INSTANCE_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(nodeInfo.n1, node), node);
            }
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_WITHOUT_DEFAULT_IMP_ADDED2,
                node->TestAttr(Attribute::DEFAULT) && !dsl.HasUnimplementedDecl(nodeInfo.n1, node), node);
        } else if (node->astKind == ASTKind::PROP_DECL) {
            if (dsl.IsStatic(node)) {
                CHECK(RuleKind::INTERFACE_STATIC_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(nodeInfo.n1, node), node);
            } else {
                CHECK(RuleKind::INTERFACE_INSTANCE_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(nodeInfo.n1, node), node);
            }
            CHECK(RuleKind::INTERFACE_MEMBER_PROP_WITHOUT_DEFAULT_IMP_ADDED2,
                node->TestAttr(Attribute::DEFAULT) && !dsl.HasUnimplementedDecl(nodeInfo.n1, node), node);
        }
    }
    if (CheckInterfaceMemberFuncOrder(id1memberDecls, id2memberDecls)) {
        CHECK(RuleKind::INTERFACE_INSTANCE_MEMBER_FUNCS_ORDER_CHANGED, false, nodeInfo.n1);
    }
}

static void CheckExtendMemberFunc(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& nodeInfo)
{
    auto e1 = nodeInfo.n1;
    auto diff = *nodeInfo.diff;
    BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(e1), dsl.Func(f1) && diff.ModuleVisible(f1));
        auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(e1));
        if (!f2) {
            continue;
        }
        if (dsl.IsConst(f1) ^ dsl.IsConst(f2)) {
            // The modifier 'const' of member function has been deleted.
            CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_CONST_DELETED, dsl.IsConst(f2), f1, f2);
            if (IsClassExtendedType(e1) && !dsl.IsStatic(f1) && !dsl.IsStatic(f2)) {
                // The modifier 'const' of member function has been added.
                CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_CONST_ADDED, dsl.IsConst(f1), f1, f2);
            }
        }
        if (IsStructExtendedType(e1) && !dsl.IsStatic(f1) && !dsl.IsStatic(f2)) {
            // The modifier 'mut' of member function has been added.
            CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_MUT_ADDED, !IsAttributeMutAdded(f1, f2), f1, f2);
            // The modifier 'mut' of member function has been deleted.
            CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_MUT_DELETED, !IsAttributeMutDeleted(f1, f2), f1, f2);
        }
        // The modifier 'public' of member function has been deleted.
        if (IsClassExtendedType(e1)) {
            auto visibility = (dsl.IsPublicOrProtected(f1) && !dsl.IsPublicOrProtected(f2)) ||
                (dsl.IsPublic(f1) && dsl.IsProtected(f2));
            CHECK(RuleKind::EXTEND_MEMBER_FUNC_VISIBILITY_CHANGED, !visibility, f1, f2);
        } else {
            if (dsl.IsStatic(f1)) {
                auto visibility = dsl.IsPublic(f1) && !dsl.IsPublic(f2) && !dsl.IsProtected(f2);
                CHECK(RuleKind::EXTEND_MEMBER_FUNC_VISIBILITY_CHANGED, !visibility, f1, f2);
            } else {
                CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_PUBLIC_DELETED,
                    !(dsl.IsPublic(f1) && !dsl.IsPublic(f2)), f1, f2);
            }
        }

        // The modifier 'static' of member function has been added.
        CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_STATIC_ADDED, !(!dsl.IsStatic(f1) && dsl.IsStatic(f2)), f1, f2);
        // The modifier 'static' of member function has been deleted.
        CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_STATIC_DELETED, !(dsl.IsStatic(f1) && !dsl.IsStatic(f2)), f1, f2);
        // The modifier 'unsafe' of member function has been added.
        CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_UNSAFE_ADDED, !IsAttributeUnsafeAdded(f1, f2), f1, f2);
        // The annotation '@Frozen' of member function has been deleted.
        CHECK(RuleKind::EXTEND_MEMBER_FUNC_FROZEN_DELETED, !IsFrozenAnnotationDeleted(dsl, f1, f2), f1, f2);
        // The annotation(Deprecated) of member function has been added or changed to strict.
        CHECK(RuleKind::EXTEND_MEMBER_FUNC_DEPRECATED_CHANGED, !IsDepreAnnotationChanged(dsl, f1, f2), f1, f2);

        NodeInfo funcInfo{f1, f2};
        // The return type of member function has been changed.
        CheckFuncReturnType(dsl, logger, checker, checkerResult, funcInfo);

        if (dsl.IsConstOrFrozenFunc(f1, f2)) {
            CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, dsl.BodyHashOf(f1) == dsl.BodyHashOf(f2), f1, f2);
        }
    END_FORALL()
}

static void CheckExtendMember(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& nodeInfo)
{
    auto e1 = nodeInfo.n1;
    auto diff = *nodeInfo.diff;
    CheckMemberFuncParams(dsl, logger, checker, checkerResult, nodeInfo);
    BEGIN_FORALL(f1, diff.GetMemberDeleted(e1), dsl.Func(f1) && diff.ModuleVisible(f1));
        auto hasSameFunc = false;
        BEGIN_FORALL(f2, diff.GetMemberAdded(e1), dsl.Func(f2) && dsl.SameFunc(f1, f2));
            // The modifier(const) of member function has been deleted.
            CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_CONST_DELETED,
                !(dsl.IsConst(f1) && !dsl.IsConst(f2)), f1, f2);
            if (IsClassExtendedType(e1) && !dsl.IsStatic(f1) && !dsl.IsStatic(f2)) {
                CHECK(RuleKind::EXTEND_MEMBER_FUNC_MODIFIER_CONST_ADDED,
                    !(!dsl.IsConst(f1) && dsl.IsConst(f2)), f1, f2);
            }
            if (dsl.IsConstOrFrozenFunc(f1, f2)) {
                CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, dsl.BodyHashOf(f1) == dsl.BodyHashOf(f2), f1, f2);
            }
            if (IsClassExtendedType(e1) && (dsl.IsPublic(f1) || dsl.IsProtected(f1) || dsl.IsOpen(f1))) {
                CHECK(RuleKind::EXTEND_MEMBER_FUNC_FROZEN_DELETED, !IsFrozenAnnotationDeleted(dsl, f1, f2), f1, f2);
            }
            if (IsStructExtendedType(e1) && dsl.IsPublic(f1)) {
                CHECK(RuleKind::EXTEND_MEMBER_FUNC_FROZEN_DELETED, !IsFrozenAnnotationDeleted(dsl, f1, f2), f1, f2);
            }
            // The return type of member function has been changed.
            NodeInfo funcInfo{f1, f2};
            CheckFuncReturnType(dsl, logger, checker, checkerResult, funcInfo);
            hasSameFunc = true;
        END_FORALL()
        BEGIN_FORALL(f2, diff.GetMemberAdded(e1),
            dsl.Func(f2) && diff.ModuleVisible(f2) && dsl.SameIdentifier(f1, f2));
            hasSameFunc = true;
        END_FORALL()
        if (dsl.IsConst(f1) && f1->TestAttr(Attribute::PRIVATE) && !hasSameFunc) {
            CHECK(RuleKind::EXTEND_PRIVATE_MEMBER_FUNC_CHANGED, false, f1);
        } else {
            // The member function visible outside the module has been deleted.
            if (IsClassExtendedType(e1)) {
                CHECK(RuleKind::EXTEND_MEMBER_FUNC_DELETED1, hasSameFunc, f1);
            }
            if (IsStructExtendedType(e1)) {
                CHECK(RuleKind::EXTEND_MEMBER_FUNC_DELETED2, hasSameFunc, f1);
            }
        }
    END_FORALL()
    // ABI Incompatible: add member function with default implementation.
    BEGIN_FORALL(f1, diff.GetMemberAdded(e1), dsl.Func(f1));
        auto hasSameFunc = false;
        BEGIN_FORALL(f2, diff.GetMemberDeleted(e1), dsl.Func(f2) && dsl.SameFunc(f1, f2));
            hasSameFunc = true;
        END_FORALL()
        if (hasSameFunc) {
            continue;
        }
        if (dsl.IsStatic(f1)) {
            CHECK(RuleKind::EXTEND_STATIC_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(e1, f1), f1);
        } else {
            CHECK(RuleKind::EXTEND_INSTANCE_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(e1, f1), f1);
        }
    END_FORALL()
    CheckExtendMemberFunc(dsl, logger, checker, checkerResult, nodeInfo);
}
}

bool CheckerImpl::IsCheckedAlready(std::vector<Node*>& vec, Node* n)
{
    for (auto ptr : vec) {
        if (ptr == n) {
            return true;
        }
    }
    return false;
}

bool CheckerImpl::ChangeFuncParamOrder(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo)
{
    auto f1 = funcInfo.n1;
    auto f2 = funcInfo.n2;
    if (!dsl.SameNumberParams(f1, f2)) {
        return false;
    }
    auto numParams = dsl.NumFuncParams(f1);
    size_t f1NamedParamCnt = 0;
    size_t f2NamedParamCnt = 0;
    std::set<std::string> f1NamedParamNames;
    std::set<std::string> f2NamedParamNames;
    std::set<std::string> f1UnnamedParamNames;
    std::set<std::string> f2UnnamedParamNames;
    bool changedNamedParam = false;
    bool changedUnnamedParam = false;
    BEGIN_FORALL(i, dsl.Range(0, numParams), true)
        auto p1 = dsl.GetFuncParam(f1, i);
        auto p2 = dsl.GetFuncParam(f2, i);
        if (!p1 || !p2) {
            continue;
        }
        auto p1NameWithType = dsl.NameOfParam(p1) + Ty::ToString(dsl.TypeOf(p1));
        auto p2NameWithType = dsl.NameOfParam(p2) + Ty::ToString(dsl.TypeOf(p2));
        if (dsl.IsNamedParam(p1)) {
            f1NamedParamNames.insert(p1NameWithType);
        } else {
            f1UnnamedParamNames.insert(p1NameWithType);
        }
        if (dsl.IsNamedParam(p2)) {
            if (!changedNamedParam) {
                changedNamedParam = dsl.NameOfParam(p1) != dsl.NameOfParam(p2);
            }
            f2NamedParamNames.insert(p2NameWithType);
        } else {
            if (!changedUnnamedParam) {
                changedUnnamedParam = dsl.NameOfParam(p1) != dsl.NameOfParam(p2);
            }
            f2UnnamedParamNames.insert(p2NameWithType);
        }
    END_FORALL()
    bool changeOrder = false;
    if (changedNamedParam && (f1NamedParamNames == f2NamedParamNames)) {
        CHECK(RuleKind::FUNC_CHANGE_ORDER_OF_NAMED_PARAMETER, false, f1, f2);
        changeOrder = true;
    }
    if (changedUnnamedParam && (f1UnnamedParamNames == f2UnnamedParamNames)) {
        CHECK(RuleKind::FUNC_CHANGE_ORDER_OF_UNNAMED_PARAMETER, false, f1, f2);
        changeOrder = true;
    }
    if (changeOrder) {
        return true;
    }
    return false;
}

void CheckerImpl::CheckFuncParams(
    Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo)
{
    auto f1 = funcInfo.n1;
    auto f2 = funcInfo.n2;
    // The parameter of function has been deleted or added.
    CHECK(RuleKind::FUNC_NUM_PARAMS, dsl.SameNumberParams(f1, f2), f1, f2);
    auto numParams = dsl.NumFuncParams(f1);
    BEGIN_FORALL(i, dsl.Range(0, numParams), dsl.SameNumberParams(f1, f2))
        auto p1 = dsl.GetFuncParam(f1, i);
        auto p2 = dsl.GetFuncParam(f2, i);
        if (!p1 || !p2) {
            continue;
        }
        auto sameName = dsl.NameOfParam(p1) == dsl.NameOfParam(p2);
        auto sameType = dsl.SameType(p1->GetTy(), p2->GetTy());
        if (dsl.IsNamedParam(p1)) {
            // The named parameter of function has been changed to unnamed parameter.
            CHECK(RuleKind::FUNC_PARAMETER_NAMED_TO_UNNAMED, dsl.IsNamedParam(p2), f1, f2, p1, p2);
            // The named parameter's name of function has been changed.
            CHECK(RuleKind::FUNC_CHANGE_NAME_OF_NAMED_PARAMETER, sameName, f1, f2, p1, p2);
            // Delete the default value of a parameter.
            CHECK(RuleKind::FUNC_DROP_DEFAUT_VALUE_OF_NAMED_PARAMETER,
                !(dsl.HasDefaultValue(p1) && !dsl.HasDefaultValue(p2)), f1, f2, p1, p2);
        } else {
            // The unnamed parameter of function has been changed to named parameter.
            CHECK(RuleKind::FUNC_PARAMETER_UNNAMED_TO_NAMED, !dsl.IsNamedParam(p2), f1, f2, p1, p2);
        }
        if (sameName && sameType) {
            // The default value of function parameter has been changed.
            if (dsl.IsConstOrFrozenFunc(f1, f2) && dsl.HasDefaultValue(p1) && dsl.HasDefaultValue(p2)) {
                CHECK(RuleKind::FUNC_PARAMETER_VALUE_CHANGE_OF_CONST_OR_FROZEN,
                    dsl.BodyHashOf(p1) == dsl.BodyHashOf(p2), p1, p2);
            }
            if (AddMemberParamDefaultValue(dsl, f1, p1, p2)) {
                CHECK(RuleKind::FUNC_MEMBER_PARAMETER_VALUE_ADD_OF_CONST_OR_FROZEN, false, p1, p2);
            }
        }
        if (dsl.IsMemberParam(p1) && dsl.IsMemberParam(p2)) {
            // The member parameter's type of constructor has been changed.
            if (sameName && !sameType) {
                CHECK(RuleKind::FUNC_CHANGE_TYPE_OF_MEMBER_PARAMETER, false, f1, f2, p1, p2);
            }
            continue;
        }
        if ((dsl.IsNamedParam(p1) == dsl.IsNamedParam(p2)) && !sameType) {
            // The parameter's type of open member function has been changed.
            if (IsClassOpenInstanceFunc(dsl, f1)) {
                CHECK(RuleKind::FUNC_CHANGE_TYPE_OF_PARAMETER_OPEN, false, f1, f2, p1, p2);
                continue;
            }
            // Change the parameter type from A to B.
            // API: compatible when B is the parent type of A; conversely, incompatible. ABI: incompatible.
            if (dsl.IsParentType(p2->GetTy(), p1->GetTy())) {
                CHECK(RuleKind::FUNC_CHANGE_TYPE_OF_PARAMETER, false, f1, f2, p1, p2);
            } else {
                CHECK(RuleKind::FUNC_CHANGE_TYPE_OF_PARAMETER_NOT_PARENT_TYPE, false, f1, f2, p1, p2);
            }
        }
    END_FORALL()
}

bool CheckerImpl::StructMemberFuncOrPropAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDomPotentiallyModified(), dsl.StructDecl(s1) && diff.ModuleVisible(s1));
        BEGIN_FORALL(f1, diff.GetMemberAdded(s1), dsl.Func(f1));
            auto hasSameFunc = false;
            BEGIN_FORALL(f2, diff.GetMemberDeleted(s1), dsl.Func(f2) && dsl.SameFunc(f1, f2));
                hasSameFunc = true;
            END_FORALL()
            if (hasSameFunc) {
                continue;
            }
            if (dsl.IsStatic(f1)) {
                CHECK(RuleKind::STRUCT_STATIC_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(s1, f1), f1);
            } else {
                CHECK(RuleKind::STRUCT_INSTANCE_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(s1, f1), f1);
            }
        END_FORALL()
        BEGIN_FORALL(p1, diff.GetMemberAdded(s1), dsl.PropDecl(p1));
            auto hasSameProp = false;
            BEGIN_FORALL(p2, diff.GetMemberDeleted(s1), dsl.PropDecl(p2) && dsl.SameIdentifier(p1, p2));
                hasSameProp = true;
            END_FORALL()
            if (hasSameProp) {
                continue;
            }
            if (dsl.IsStatic(p1)) {
                CHECK(RuleKind::STRUCT_STATIC_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(s1, p1), p1);
            } else {
                CHECK(RuleKind::STRUCT_INSTANCE_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(s1, p1), p1);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructMemberFuncDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDomPotentiallyModified(),
        dsl.StructDecl(s1) && dsl.TopLevel(s1) && diff.ModuleVisible(s1));
        LETIF(s2, dsl.Corresponding(s1, diff.PotentiallyModified()), dsl.TopLevel(s2) && diff.ModuleVisible(s2))
        NodeInfo nodeInfo{s1, s2, &diff};
        CheckMemberFuncParams(dsl, logger, checker, checkerResult, nodeInfo);
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(s1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(s1));
            if (!f2) {
                continue;
            }
            auto ruleKind = f1->TestAnyAttr(Attribute::CONSTRUCTOR) ?
                RuleKind::STRUCT_INIT_FUNC_MODIFIER_CHANGED : RuleKind::STRUCT_MEMBER_FUNC_MODIFIER_CHANGED;
            if (dsl.IsPublic(f1) && !dsl.IsPublic(f2)) {
                // The modifier(public) of public member function has been deleted.
                CHECK(ruleKind, false, f1, f2);
            }
            // The modifier(const/static) of member function has been deleted.
            CHECK(ruleKind, !isAttributeConstOrStaticDeleted(dsl, f1, f2), f1, f2);
            // The modifier(unsafe/mut/static) of member function has been added.
            CHECK(ruleKind, !IsAttributeUnsafeOrMutOrStaticAdded(f1, f2), f1, f2);
            // The modifier(mut) of member function has been deleted.
            ruleKind = f1->TestAnyAttr(Attribute::CONSTRUCTOR) ?
                RuleKind::STRUCT_INIT_FUNC_MODIFIER_CHANGED2 : RuleKind::STRUCT_MEMBER_FUNC_MODIFIER_CHANGED2;
            CHECK(ruleKind, !IsAttributeMutDeleted(f1, f2), f1, f2);
            // The annotation(Deprecated) of member function has been added or changed to strict.
            ruleKind = f1->TestAnyAttr(Attribute::CONSTRUCTOR) ?
                RuleKind::STRUCT_INIT_FUNC_ANNO_CHANGED : RuleKind::STRUCT_MEMBER_FUNC_ANNO_CHANGED;
            CHECK(ruleKind, !IsDepreAnnotationChanged(dsl, f1, f2), f1, f2);
            // The annotation(Frozen) of member function has been deleted.
            ruleKind = f1->TestAnyAttr(Attribute::CONSTRUCTOR) ?
                RuleKind::STRUCT_INIT_FUNC_ANNO_CHANGED2 : RuleKind::STRUCT_MEMBER_FUNC_ANNO_CHANGED2;
            CHECK(ruleKind, !IsFrozenAnnotationDeleted(dsl, f1, f2), f1, f2);
            NodeInfo funcInfo{f1, f2};
            // The return type of member function has been changed.
            CheckFuncReturnType(dsl, logger, checker, checkerResult, funcInfo);
            if (dsl.IsConstOrFrozenFunc(f1, f2)) {
                CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, dsl.BodyHashOf(f1) == dsl.BodyHashOf(f2), f1, f2);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::StructMemberPropDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(s1, diff.GetDomPotentiallyModified(),
        dsl.StructDecl(s1) && dsl.TopLevel(s1) && diff.ModuleVisible(s1));
        LETIF(s2, dsl.Corresponding(s1, diff.PotentiallyModified()),
            dsl.TopLevel(s2) && diff.ModuleVisible(s2))
        BEGIN_FORALL(p1, diff.GetMemberDeleted(s1), dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            auto hasSameFunc = false;
            BEGIN_FORALL(p2, diff.GetMemberAdded(s1),
                dsl.PropDecl(p2) && diff.ModuleVisible(p2) && dsl.SameIdentifier(p1, p2));
                hasSameFunc = true;
            END_FORALL()
            // The member property has been deleted.
            CHECK(RuleKind::STRUCT_MEMBER_PROP_DELETED, hasSameFunc, p1);
        END_FORALL()
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(s1), dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            LETIF(p2, dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(s1)), dsl.PropDecl(p2))
            if (dsl.IsPublic(p1) && !dsl.IsPublic(p2)) {
                // The modifier(public) of public member property has been deleted.
                CHECK(RuleKind::STRUCT_MEMBER_PROP_MODIFIER_CHANGED, false, p1, p2);
            }
            if (dsl.IsStatic(p1) && dsl.IsStatic(p2)) {
                // The modifier(mut/static) of member property has been changed.
                CHECK(RuleKind::STRUCT_MEMBER_PROP_MODIFIER_CHANGED, !IsAttributeMutDeleted(p1, p2), p1, p2);
            } else {
                // The modifier(mut/static) of member property has been changed.
                auto propModifierChanged = (IsAttributeUnsafeOrMutOrStaticAdded(p1, p2)
                    || IsAttributeMutDeleted(p1, p2)) || isAttributeConstOrStaticDeleted(dsl, p1, p2);
                CHECK(RuleKind::STRUCT_MEMBER_PROP_MODIFIER_CHANGED, !propModifierChanged, p1, p2);
            }
            // The annotation(Deprecated) of member property has been changed to strict.
            CHECK(RuleKind::STRUCT_MEMBER_PROP_ANNO_CHANGED, !IsDepreAnnotationChanged(dsl, p1, p2), p1, p2);
            // The annotation(Frozen) of member property has been deleted.
            CHECK(RuleKind::STRUCT_MEMBER_PROP_ANNO_CHANGED2,
                !IsFrozenAnnotationDeleted(dsl, p1, p2), p1, p2);
            // The return type of member property has been changed.
            CHECK(RuleKind::STRUCT_MEMBER_PROP_RETURN_TYPE_CHANGED, dsl.SameType(p1->GetTy(), p2->GetTy()), p1, p2);
            // The funcbody of getter/setter in member property has been changed.
            NodeInfo propInfo{p1, p2};
            CheckPropBodyChanged(dsl, logger, checker, checkerResult, propInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumMemberFuncOrPropAdded(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(), dsl.EnumDecl(e1) && diff.ModuleVisible(e1));
        BEGIN_FORALL(f1, diff.GetMemberAdded(e1), dsl.Func(f1));
            auto hasSameFunc = false;
            BEGIN_FORALL(f2, diff.GetMemberDeleted(e1), dsl.Func(f2) && dsl.SameFunc(f1, f2));
                hasSameFunc = true;
            END_FORALL()
            if (hasSameFunc) {
                continue;
            }
            if (dsl.IsStatic(f1)) {
                CHECK(RuleKind::ENUM_STATIC_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(e1, f1), f1);
            } else {
                CHECK(RuleKind::ENUM_INSTANCE_MEMBER_FUNC_ADDED, !dsl.HasOverridingDecl(e1, f1), f1);
            }
        END_FORALL()
        BEGIN_FORALL(p1, diff.GetMemberAdded(e1), dsl.PropDecl(p1));
            auto hasSameProp = false;
            BEGIN_FORALL(p2, diff.GetMemberDeleted(e1), dsl.PropDecl(p2) && dsl.SameIdentifier(p1, p2));
                hasSameProp = true;
            END_FORALL()
            if (hasSameProp) {
                continue;
            }
            if (dsl.IsStatic(p1)) {
                CHECK(RuleKind::ENUM_STATIC_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(e1, p1), p1);
            } else {
                CHECK(RuleKind::ENUM_INSTANCE_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(e1, p1), p1);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumMemberFuncDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(),
        dsl.EnumDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        LETIF(e2, dsl.Corresponding(e1, diff.PotentiallyModified()), dsl.TopLevel(e2) && diff.ModuleVisible(e2))
        NodeInfo nodeInfo{e1, e2, &diff};
        CheckMemberFuncParams(dsl, logger, checker, checkerResult, nodeInfo);
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(e1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(e1));
            if (!f2) {
                continue;
            }
            if (dsl.IsPublic(f1) && !dsl.IsPublic(f2)) {
                // The modifier(public) of public member function has been deleted.
                CHECK(RuleKind::ENUM_MEMBER_FUNC_MODIFIER_CHANGED, false, f1, f2);
            }
            // The modifier(const/static) of member function has been deleted.
            CHECK(RuleKind::ENUM_MEMBER_FUNC_MODIFIER_CHANGED, !isAttributeConstOrStaticDeleted(dsl, f1, f2), f1, f2);
            // The modifier(unsafe/mut) of member function has been added.
            CHECK(RuleKind::ENUM_MEMBER_FUNC_MODIFIER_CHANGED, !IsAttributeUnsafeOrMutOrStaticAdded(f1, f2), f1, f2);
            // The annotation(Deprecated) of member function has been added or changed to strict.
            CHECK(RuleKind::ENUM_MEMBER_FUNC_ANNO_CHANGED, !IsDepreAnnotationChanged(dsl, f1, f2), f1, f2);
            // The annotation(Frozen) of member function has been deleted.
            CHECK(RuleKind::ENUM_MEMBER_FUNC_ANNO_CHANGED2, !IsFrozenAnnotationDeleted(dsl, f1, f2), f1, f2);
            NodeInfo funcInfo{f1, f2};
            // The return type of member function has been changed.
            CheckFuncReturnType(dsl, logger, checker, checkerResult, funcInfo);
            if (dsl.IsConstOrFrozenFunc(f1, f2)) {
                CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, dsl.BodyHashOf(f1) == dsl.BodyHashOf(f2), f1, f2);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::EnumMemberPropDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(),
        dsl.EnumDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        LETIF(e2, dsl.Corresponding(e1, diff.PotentiallyModified()),
            dsl.TopLevel(e2) && diff.ModuleVisible(e2))
        BEGIN_FORALL(p1, diff.GetMemberDeleted(e1), dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            auto hasSameFunc = false;
            BEGIN_FORALL(p2, diff.GetMemberAdded(e1),
                dsl.PropDecl(p2) && diff.ModuleVisible(p2) && dsl.SameIdentifier(p1, p2));
                hasSameFunc = true;
            END_FORALL()
            // The member property has been deleted.
            CHECK(RuleKind::ENUM_MEMBER_PROP_DELETED, hasSameFunc, p1);
        END_FORALL()
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(e1), dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            LETIF(p2, dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(e1)), dsl.PropDecl(p2))
            if (dsl.IsPublic(p1) && !dsl.IsPublic(p2)) {
                // The modifier(public) of public member property has been deleted.
                CHECK(RuleKind::ENUM_MEMBER_PROP_MODIFIER_CHANGED, false, p1, p2);
            }
            // The modifier(static) of member property has been changed.
            auto modifierChanged = (dsl.IsStatic(p1) && !dsl.IsStatic(p2))
                || (!dsl.IsStatic(p1) && dsl.IsStatic(p2));
            CHECK(RuleKind::ENUM_MEMBER_PROP_MODIFIER_CHANGED, !modifierChanged, p1, p2);
            // The annotation(Deprecated) of member property has been changed to strict.
            CHECK(RuleKind::ENUM_MEMBER_PROP_ANNO_CHANGED, !IsDepreAnnotationChanged(dsl, p1, p2), p1, p2);
            // The annotation(Frozen) of member property has been deleted.
            CHECK(RuleKind::ENUM_MEMBER_PROP_ANNO_CHANGED2, !IsFrozenAnnotationDeleted(dsl, p1, p2), p1, p2);
            // The return type of member property has been changed.
            CHECK(RuleKind::ENUM_MEMBER_PROP_RETURN_TYPE_CHANGED, dsl.SameType(p1->GetTy(), p2->GetTy()), p1, p2);
            // The funcbody of getter/setter in member property has been changed.
            NodeInfo propInfo{p1, p2};
            CheckPropBodyChanged(dsl, logger, checker, checkerResult, propInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberFuncParamsOrRetTypeChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()),
            dsl.ClassDecl(c2) && dsl.TopLevel(c2) && diff.ModuleVisible(c2))
        NodeInfo nodeInfo{c1, c2, &diff};
        CheckMemberFuncParams(dsl, logger, checker, checkerResult, nodeInfo);
        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(c1), dsl.Func(f1) && diff.ModuleVisible(f1))
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(c1));
            NodeInfo funcInfo{f1, f2};
            // The return type of member function has been changed.
            CheckFuncReturnType(dsl, logger, checker, checkerResult, funcInfo);
            if (!dsl.IsConstOrFrozenFunc(f1, f2)) {
                continue;
            }
            CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, dsl.BodyHashOf(f1) == dsl.BodyHashOf(f2), f1, f2);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ClassInstanceMemberPropRetTypeOrBodyChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(c1, diff.GetDomPotentiallyModified(),
        dsl.ClassDecl(c1) && dsl.TopLevel(c1) && diff.ModuleVisible(c1))
        LETIF(c2, dsl.Corresponding(c1, diff.PotentiallyModified()), dsl.ClassDecl(c2) && diff.ModuleVisible(c2))
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(c1), dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            LETIF(p2, dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(c1)), dsl.PropDecl(p2))
            // The return type of member property has been changed.
            CHECK(RuleKind::CLASS_INSTANCE_MEMBER_PROP_RETURN_TYPE_CHANGED, dsl.SameType(p1->GetTy(), p2->GetTy()), p1, p2);
            // The funcbody of getter/setter in member property has been changed.
            NodeInfo propInfo{p1, p2};
            CheckPropBodyChanged(dsl, logger, checker, checkerResult, propInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceMemberFuncAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(i1, diff.GetDomPotentiallyModified(),
        dsl.InterfaceDecl(i1) && dsl.TopLevel(i1) && diff.ModuleVisible(i1));
        LETIF(i2, dsl.Corresponding(i1, diff.PotentiallyModified()), dsl.TopLevel(i2) && diff.ModuleVisible(i2))
        NodeInfo nodeInfo{i1, i2, &diff};
        CheckMemberFuncParams(dsl, logger, checker, checkerResult, nodeInfo);
        CheckInterfaceMemberFuncOrderAndAdded(dsl, logger, checker, checkerResult, nodeInfo);

        BEGIN_FORALL(f1, diff.GetDomPotentiallyMemberModified(i1), dsl.Func(f1) && diff.ModuleVisible(f1));
            auto f2 = dsl.Corresponding(f1, diff.GetPotentiallyMemberModified(i1));
            if (!f2) {
                continue;
            }
            if (dsl.IsConst(f1) ^ dsl.IsConst(f2)) {
                // The modifier 'const' of member function has been deleted.
                CHECK(RuleKind::INTERFACE_MEMBER_FUNC_MODIFIER_CONST_DELETED, dsl.IsConst(f2), f1, f2);
                // The modifier 'const' of member function has been added.
                CHECK(RuleKind::INTERFACE_MEMBER_FUNC_MODIFIER_CONST_ADDED, dsl.IsConst(f1), f1, f2);
            }
            // The modifier 'static' of member function has been deleted.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_PROP_MODIFIER_STATIC_DELETED,
                !(dsl.IsStatic(f1) && !dsl.IsStatic(f2)), f1, f2);
            // The modifier 'static' of member function has been added.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_PROP_MODIFIER_STATIC_ADDED,
                !(!dsl.IsStatic(f1) && dsl.IsStatic(f2)), f1, f2);
            // The modifier 'mut' of member function has been deleted.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_PROP_MODIFIER_MUT_DELETED, !IsAttributeMutDeleted(f1, f2), f1, f2);
            // The modifier 'mut' of member function has been added.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_MODIFIER_MUT_ADDED, !IsAttributeMutAdded(f1, f2), f1, f2);
            // The modifier 'unsafe' of member function has been added.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_MODIFIER_UNSAFE_ADDED, !IsAttributeUnsafeAdded(f1, f2), f1, f2);
            // The annotation '@Frozen' of member function has been deleted.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_FROZEN_DELETED, !IsFrozenAnnotationDeleted(dsl, f1, f2), f1, f2);
            // The annotation(Deprecated) of member function has been added or changed to strict.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_DEPRECATED_CHANGED, !IsDepreAnnotationChanged(dsl, f1, f2), f1, f2);

            // The return type of member function has been changed.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_RETURN_TYPE,
                dsl.SameType(dsl.ReturnType(f1), dsl.ReturnType(f2)), f1, f2);

            if (dsl.IsConstOrFrozenFunc(f1, f2)) {
                CHECK(RuleKind::CONST_OR_FROZEN_BODY_CHANGE, dsl.BodyHashOf(f1) == dsl.BodyHashOf(f2), f1, f2);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::InterfaceMemberPropAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(i1, diff.GetDomPotentiallyModified(),
        dsl.InterfaceDecl(i1) && dsl.TopLevel(i1) && diff.ModuleVisible(i1));
        BEGIN_FORALL(p1, diff.GetMemberDeleted(i1), dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            // The member property has been deleted.
            CHECK(RuleKind::INTERFACE_MEMBER_PROP_DELETED, false, p1);
        END_FORALL()
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(i1),
            dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            LETIF(p2, dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(i1)), dsl.PropDecl(p2))
            // The modifier 'static' of member property has been deleted.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_PROP_MODIFIER_STATIC_DELETED,
                !(dsl.IsStatic(p1) && !dsl.IsStatic(p2)), p1, p2);
            // The modifier 'static' of member property has been added.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_PROP_MODIFIER_STATIC_ADDED,
                !(!dsl.IsStatic(p1) && dsl.IsStatic(p2)), p1, p2);
            // The modifier 'mut' of member property has been deleted.
            CHECK(RuleKind::INTERFACE_MEMBER_FUNC_PROP_MODIFIER_MUT_DELETED, !IsAttributeMutDeleted(p1, p2), p1, p2);
            // The modifier 'mut' of member property has been added.
            CHECK(RuleKind::INTERFACE_MEMBER_PROP_MODIFIER_MUT_ADDED, !IsAttributeMutAdded(p1, p2), p1, p2);
            // The annotation(Deprecated) of member property has been changed to strict.
            CHECK(RuleKind::INTERFACE_MEMBER_PROP_DEPRECATED_CHANGED, !IsDepreAnnotationChanged(dsl, p1, p2), p1, p2);
            // The annotation(Frozen) of member property has been deleted.
            CHECK(RuleKind::INTERFACE_MEMBER_PROP_FROZEN_DELETED, !IsFrozenAnnotationDeleted(dsl, p1, p2), p1, p2);
            // The return type of member property has been changed.
            CHECK(RuleKind::INTERFACE_MEMBER_PROP_RETURN_TYPE_CHANGED, dsl.SameType(p1->GetTy(), p2->GetTy()), p1, p2);
            // The funcbody of getter/setter in member property has been changed.
            NodeInfo propInfo{p1, p2};
            CheckPropBodyChanged(dsl, logger, checker, checkerResult, propInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ExtendMemberFuncDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(), dsl.ExtendDecl(e1) && diff.ModuleVisible(e1));
        LETIF(e2, dsl.Corresponding(e1, diff.PotentiallyModified()), diff.ModuleVisible(e2))
        NodeInfo extendInfo{e1, e2, &diff};
        CheckExtendMember(dsl, logger, checker, checkerResult, extendInfo);
    END_FORALL()
    BEGIN_FORALL(e1, diff.GetDeleted(), dsl.ExtendDecl(e1) && diff.ModuleVisible(e1));
        BEGIN_FORALL(e2, diff.GetAdded(),
            dsl.ExtendDecl(e2) && diff.ModuleVisible(e2) && diff.SameExtend(e1, e2));
            NodeInfo extendInfo{e1, e2, &diff};
            CheckExtendMember(dsl, logger, checker, checkerResult, extendInfo);
        END_FORALL()
    END_FORALL()
    return checkerResult;
}

bool CheckerImpl::ExtendMemberPropAddedDeletedOrChanged(
    Dsl& dsl, const Diff& diff, Logger& logger, const Checker& checker)
{
    auto checkerResult{true};
    BEGIN_FORALL(e1, diff.GetDomPotentiallyModified(),
        dsl.ExtendDecl(e1) && dsl.TopLevel(e1) && diff.ModuleVisible(e1));
        BEGIN_FORALL(p1, diff.GetMemberDeleted(e1), dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            // The member property has been deleted.
            CHECK(RuleKind::EXTEND_MEMBER_PROP_DELETED, false, p1);
        END_FORALL()
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(e1), dsl.PropDecl(p1) && diff.ModuleVisible(p1));
            LETIF(p2, dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(e1)), dsl.PropDecl(p2))
            if (dsl.IsStatic(p1)) {
                CHECK(RuleKind::EXTEND_MEMBER_PROP_MODIFIER_PUBLIC_DELETED,
                    !(dsl.IsPublic(p1) && !dsl.IsPublic(p2)), p1, p2);
                CHECK(RuleKind::EXTEND_MEMBER_PROP_MODIFIER_STATIC_DELETED, dsl.IsStatic(p2), p1, p2);
            } else {
                // The modifier 'public' of member function has been deleted.
                if (IsStructExtendedType(e1)) {
                    CHECK(RuleKind::EXTEND_MEMBER_PROP_MODIFIER_PUBLIC_DELETED,
                        !(dsl.IsPublic(p1) && !dsl.IsPublic(p2)), p1, p2);
                } else {
                    auto modifyPublic = (dsl.IsPublicOrProtected(p1) && !dsl.IsPublicOrProtected(p2)) ||
                        (dsl.IsPublic(p1) && dsl.IsProtected(p2));
                    CHECK(RuleKind::EXTEND_MEMBER_PROP_VISIBILITY_CHANGED, !modifyPublic, p1, p2);
                }
            }
            // The static of member property has been added.
            CHECK(RuleKind::EXTEND_MEMBER_PROP_MODIFIER_STATIC_ADDED, !isAttributeConstOrStaticAdded(dsl, p1, p2), p1,
                p2);
            if (!IsClassExtendedType(e1) && !dsl.IsStatic(p1) && !dsl.IsStatic(p2)) {
                // The modifier 'mut' of member property has been added.
                CHECK(RuleKind::EXTEND_MEMBER_PROP_MUT_ADDED, !IsAttributeMutAdded(p1, p2), p1, p2);
            }
            // The Mut of member property has been deleted.
            CHECK(RuleKind::EXTEND_MEMBER_PROP_MUT_DELETED, !IsAttributeMutDeleted(p1, p2), p1, p2);
            // The annotation(Deprecated) of member property has been changed to strict.
            CHECK(RuleKind::EXTEND_MEMBER_PROP_DEPRECATED_CHANGED, !IsDepreAnnotationChanged(dsl, p1, p2), p1, p2);
            // The annotation(Frozen) of member property has been deleted.
            CHECK(RuleKind::EXTEND_MEMBER_PROP_FROZEN_DELETED, !IsFrozenAnnotationDeleted(dsl, p1, p2), p1, p2);
            // The return type of member property has been changed.
            CHECK(RuleKind::EXTEND_MEMBER_PROP_RETURN_TYPE_CHANGED, dsl.SameType(p1->GetTy(), p2->GetTy()), p1, p2);
            // The funcbody of getter/setter in member property has been changed.
            NodeInfo propInfo{p1, p2};
            CheckPropBodyChanged(dsl, logger, checker, checkerResult, propInfo);
        END_FORALL()
        BEGIN_FORALL(p1, diff.GetDomPotentiallyMemberModified(e1),
            IsClassExtendedType(e1) && dsl.PropDecl(p1) && dsl.IsProtected(p1));
            LETIF(p2, dsl.Corresponding(p1, diff.GetPotentiallyMemberModified(e1)), dsl.PropDecl(p2))
            // The annotation(Frozen) of member property has been deleted when extend type is class.
            CHECK(RuleKind::EXTEND_MEMBER_PROP_FROZEN_DELETED, !IsFrozenAnnotationDeleted(dsl, p1, p2), p1, p2);
        END_FORALL()
        // ABI Incompatible: add property with default implementation.
        BEGIN_FORALL(p1, diff.GetMemberAdded(e1), dsl.PropDecl(p1));
            auto hasSameFunc = false;
            BEGIN_FORALL(p2, diff.GetMemberDeleted(e1), dsl.PropDecl(p2) && dsl.SameIdentifier(p1, p2));
                hasSameFunc = true;
            END_FORALL()
            if (hasSameFunc) {
                continue;
            }
            if (dsl.IsStatic(p1)) {
                CHECK(RuleKind::EXTEND_STATIC_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(e1, p1), p1);
            } else {
                CHECK(RuleKind::EXTEND_INSTANCE_MEMBER_PROP_ADDED, !dsl.HasOverridingDecl(e1, p1), p1);
            }
        END_FORALL()
    END_FORALL()
    return checkerResult;
}
