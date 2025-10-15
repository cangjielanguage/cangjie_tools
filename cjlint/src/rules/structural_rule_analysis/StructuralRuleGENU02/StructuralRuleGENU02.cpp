// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGENU02.h"
#include <regex>

using namespace Cangjie;
using namespace AST;
using namespace Meta;
using namespace CodeCheck;

// Check whether there is an inheritance relationship between classes/interfaces.
bool StructuralRuleGENU02::CheckTyEqualityHelper(Cangjie::AST::Ty* base, Cangjie::AST::Ty* derived)
{
    if (derived->IsClassLike()) {
        // Same Class/Interface
        if (base == derived) {
            return true;
        }
        auto classDecl = static_cast<Cangjie::AST::ClassLikeDecl*>(Cangjie::AST::Ty::GetDeclOfTy(derived).get());
        // Explicit Inheritance When Class Is Declared
        for (auto& super : classDecl->inheritedTypes) {
            if (super->ty == base) {
                return true;
            }
            if (CheckTyEqualityHelper(base, super->ty)) {
                return true;
            }
        }
        // Inheritance relationship defined by extension
        for (auto& super : inheritedClassMap[derived]) {
            if (super->ty == base) {
                return true;
            }
            if (CheckTyEqualityHelper(base, super->ty)) {
                return true;
            }
        }
    }
    return false;
}

// For non-class types, the types must be the same.
// For class/interface, types must be the same or have the inheritance relationship.
bool StructuralRuleGENU02::IsEqual(Cangjie::AST::Ty* base, Cangjie::AST::Ty* derived)
{
    if (!base->IsClassLike()) {
        if (base->kind == derived->kind) {
            return true;
        } else {
            return false;
        }
    }
    return CheckTyEqualityHelper(base, derived) || CheckTyEqualityHelper(derived, base);
}

void StructuralRuleGENU02::DuplicatedEnumCtrOrFuncHelper(const Cangjie::AST::FuncDecl& funcDecl)
{
    auto& params = funcDecl.funcBody->paramLists[0]->params;
    std::vector<AST::Ty*> args;
    for (size_t i = 0; i < params.size(); i++) {
        args.emplace_back(params[i]->ty);
    }
    auto isEnumCtr = funcDecl.TestAttr(Attribute::ENUM_CONSTRUCTOR);
    auto enumCtr = EnumCtr(funcDecl.identifier, args, isEnumCtr);
    auto filter = [&enumCtr, this](const EnumCtr& item) {
        if (item.identifier != enumCtr.identifier || item.args.size() != enumCtr.args.size()) {
            return false;
        }
        for (size_t i = 0; i < item.args.size(); i++) {
            if (!IsEqual(item.args[i], enumCtr.args[i])) {
                return false;
            }
        }
        return true;
    };
    auto iter = std::find_if(enumCtrSet.begin(), enumCtrSet.end(), filter);
    if (iter != enumCtrSet.end() && (isEnumCtr || iter->isCtr)) {
        Diagnose(funcDecl.identifier.Begin(), funcDecl.identifier.End(),
            CodeCheckDiagKind::G_ENU_02_enum_constructor_overload_information, funcDecl.identifier.Val().c_str());
    } else {
        enumCtrSet.insert(enumCtrSet.end(), enumCtr);
    }
}

void StructuralRuleGENU02::CheckEnumCtrOverload(const Cangjie::AST::EnumDecl& enumDecl)
{
    for (auto& constructor : enumDecl.constructors) {
        if (constructor->astKind == ASTKind::FUNC_DECL) {
            auto funcDecl = static_cast<FuncDecl*>(constructor.get().get());
            DuplicatedEnumCtrOrFuncHelper(*funcDecl);
        }
    }
}

void StructuralRuleGENU02::CheckFuncOverload(const Cangjie::AST::FuncDecl& funcDecl)
{
    if (funcDecl.scopeLevel == 0 && !funcDecl.outerDecl) {
        DuplicatedEnumCtrOrFuncHelper(funcDecl);
    }
}

void StructuralRuleGENU02::FindEnumDeclHelper(Ptr<Node> node)
{
    if (!node) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncDecl& funcDecl) {
                CheckFuncOverload(funcDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const EnumDecl& enumDecl) {
                CheckEnumCtrOverload(enumDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGENU02::FindExtendHelper(Ptr<Cangjie::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const ExtendDecl& extendDecl) {
                if (extendDecl.extendedType->ty->IsClassLike()) {
                    for (auto& type : extendDecl.inheritedTypes) {
                        inheritedClassMap[extendDecl.extendedType->ty].emplace_back(type.get());
                    }
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGENU02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    // Collect the inheritance relationships between classes/interfaces through extendDecl
    FindExtendHelper(node);
    // Check enum's constructor and top-level functions
    FindEnumDeclHelper(node);
}
