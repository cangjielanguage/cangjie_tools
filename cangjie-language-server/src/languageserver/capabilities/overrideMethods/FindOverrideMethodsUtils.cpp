// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FindOverrideMethodsUtils.h"
#include "cangjie/Lex/Token.h"
#include "../../common/Utils.h"

namespace ark {

std::unordered_set<std::string> InitKeyWords()
{
    std::unordered_set<std::string> keyWords = {};
    for (auto &i: TOKENS) {
        std::string token(i);
        if (token.empty() || !isalpha(token[0])) { continue; }
        keyWords.insert(token);
    }
    return keyWords;
}

const std::unordered_set<std::string> KeyWords = InitKeyWords();

template <typename T>
std::unique_ptr<ClassLikeTypeDetail> ResolveGenericType(const T &t)
{
    auto detail = std::make_unique<ClassLikeTypeDetail>();
    detail->identifier = t.decl->identifier;
    if (t.decl->generic) {
        for (const auto &tyArg : t.typeArgs) {
            detail->generics.emplace_back(ResolveType(tyArg));
        }
    }
    return detail;
}

std::unique_ptr<FuncLikeTypeDetail> ResolveFuncType(const FuncTy& ty)
{
    auto detail = std::make_unique<FuncLikeTypeDetail>();
    for (const auto& paramTy: ty.paramTys) {
        detail->params.emplace_back(ResolveType(paramTy));
    }
    detail->ret = std::move(ResolveType(ty.retTy));
    return detail;
}

std::unique_ptr<VArrayTypeDetail> ResolveVarrayType(const VArrayTy& ty)
{
    auto detail = std::make_unique<VArrayTypeDetail>();
    if (ty.typeArgs.size() != 1) {
        return detail;
    }
    detail->identifier = ty.name;
    detail->tyArg = ResolveType(ty.typeArgs[0]);
    detail->size = ty.size;
    return detail;
}

std::unique_ptr<TupleTypeDetail> ResolveTupleType(const TupleTy& ty)
{
    auto detail = std::make_unique<TupleTypeDetail>();
    for (auto& typeArg: ty.typeArgs) {
        detail->params.emplace_back(ResolveType(typeArg));
    }
    return detail;
}

std::unique_ptr<TypeDetail> ResolveType(const Ptr<Ty>& type)
{
    if (!type) {
        return {};
    }
    return Meta::match(*(type)) (
        [](const ClassTy &ty) {
            std::unique_ptr<TypeDetail> ret = ResolveGenericType(ty);
            return ret;
        },
        [](const InterfaceTy &ty) {
            std::unique_ptr<TypeDetail> ret = ResolveGenericType(ty);
            return ret;
        },
        [](const EnumTy &ty) {
            std::unique_ptr<TypeDetail> ret = ResolveGenericType(ty);
            return ret;
        },
        [](const StructTy &ty) {
            std::unique_ptr<TypeDetail> ret = ResolveGenericType(ty);
            return ret;
        },
        [](const FuncTy &ty) {
            std::unique_ptr<TypeDetail> ret = ResolveFuncType(ty);
            return ret;
        },
        [](const TypeAliasTy &ty) {
            std::unique_ptr<TypeDetail> ret =
                std::move(std::make_unique<CommonTypeDetail>(ty.declPtr->identifier.Val()));
            return ret;
        },
        [](const GenericsTy &ty) {
            std::unique_ptr<TypeDetail> ret =
                std::move(std::make_unique<CommonTypeDetail>(ty.name));
            return ret;
        },
        [](const VArrayTy &ty) {
            std::unique_ptr<TypeDetail> ret = std::move(ResolveVarrayType(ty));
            return ret;
        },
        [](const TupleTy &ty) {
            std::unique_ptr<TypeDetail> ret = ResolveTupleType(ty);
            return ret;
        },
        [](const Ty &ty) {
            return std::make_unique<TypeDetail>(ty.String());
        },
        []() {
            return std::make_unique<TypeDetail>();
        }
    );
}

std::vector<std::string> ResolveDeclModifiers(const Ptr<Decl> &decl)
{
    std::vector<std::string> modifiers;
    for (const auto& item: attr2str) {
        if (decl->TestAttr(item.first)) {
            modifiers.emplace_back(item.second);
        }
    }
    return modifiers;
}

std::string ResolveDeclIdentifier(const Ptr<Decl>& decl)
{
    return decl->identifier;
}

FuncParamDetailList ResolveFuncParamList(const Ptr<FuncDecl>& funcDecl)
{
    FuncParamDetailList params;
    for (auto& paramList: funcDecl->funcBody->paramLists) {
        for (auto& param: paramList->params) {
            if (!param) return {};
            FuncParamDetail paramDetail{};
            std::string identifier = param->identifier.GetRawText();
            if (KeyWords.count(identifier)) {
                identifier = '`' + identifier + '`';
            }
            paramDetail.isNamed = param->isNamedParam;
            if (!param->ty || GetString(*param->ty) == "UnknownType") {
                continue;
            }
            paramDetail.identifier = identifier;
            paramDetail.type = std::move(ResolveType(param->ty));
            params.isVariadic = paramList->variadicArgIndex > 0;
            params.params.emplace_back(std::move(paramDetail));
        }
    }
    return params;
}

std::unique_ptr<TypeDetail> ResolveFuncRetType(const Ptr<FuncDecl>& funcDecl)
{
    if (funcDecl->funcBody == nullptr || funcDecl->funcBody->retType == nullptr) {
        return {};
    }

    auto type = funcDecl->funcBody->retType.get();
    if (!type) {
        return {};
    }
    return ResolveType(type->ty);
}

FuncDetail ResolveFuncDetail(const Ptr<FuncDecl>& funcDecl)
{
    FuncDetail detail;
    detail.modifiers = ResolveDeclModifiers(funcDecl);
    detail.identifier = ResolveDeclIdentifier(funcDecl);
    detail.params = ResolveFuncParamList(funcDecl);
    detail.retType = ResolveFuncRetType(funcDecl);
    return detail;
}

PropDetail ResolvePropDetail(const Ptr<PropDecl>& propDecl)
{
    PropDetail detail;
    detail.modifiers = ResolveDeclModifiers(propDecl);
    detail.identifier = ResolveDeclIdentifier(propDecl);
    detail.type = ResolveType(propDecl->ty);
    return detail;
}
} // namespace ark