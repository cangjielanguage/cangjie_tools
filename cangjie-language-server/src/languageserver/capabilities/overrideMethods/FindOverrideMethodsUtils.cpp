// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FindOverrideMethodsUtils.h"
#include "cangjie/Lex/Token.h"
#include "../../common/Utils.h"

namespace ark {
const std::unordered_set<TokenKind> OPERATOR_TO_OVERLOAD = {
    TokenKind::LSQUARE, TokenKind::RSQUARE, TokenKind::NOT,
    TokenKind::EXP, TokenKind::MUL, TokenKind::MOD,
    TokenKind::DIV, TokenKind::ADD, TokenKind::SUB,
    TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::LT,
    TokenKind::LE, TokenKind::GT, TokenKind::GE,
    TokenKind::EQUAL, TokenKind::NOTEQ, TokenKind::BITAND,
    TokenKind::BITXOR, TokenKind::BITOR};

const std::string TAB = "    ";
const std::string NEWLINE = "\n";
const std::string SPACE = " ";

const std::string FUNC_NOT_IMPLEMENTED_EXCEPTION = "throw Exception(\"Function not implemented.\")";
const std::string PROP_NOT_IMPLEMENTED_EXCEPTION = "throw Exception(\"Prop not implemented.\")";

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

const std::unordered_map<std::string, int> keyMap = InitKeyMap();

const std::unordered_set<std::string> KeyWords = InitKeyWords();

void FilterModifiers(Ptr<Decl> decl, std::vector<std::string>& modifiers)
{
    std::unordered_set<std::string> filterItems = {"abstract"};
    if (!decl->TestAnyAttr(Attribute::OPEN, Attribute::SEALED)) {
        filterItems.insert("open");
    }

    if (decl->astKind == ASTKind::INTERFACE_DECL) {
        filterItems.insert("public");
    }

    if (decl->astKind  == ASTKind::STRUCT_DECL || decl->astKind == ASTKind::ENUM_DECL ||
        decl->astKind == ASTKind::EXTEND_DECL) {
        filterItems.insert("open");
        filterItems.insert("override");
        filterItems.insert("redef");
    }

    if (decl->astKind == ASTKind::ENUM_DECL || decl->astKind == ASTKind::EXTEND_DECL) {
        filterItems.insert("mut");
    }

    for (auto it = modifiers.begin(); it != modifiers.end();) {
        if (filterItems.count(*it)) {
            it = modifiers.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<Ptr<ClassLikeDecl>> GetCanSuperCallDecls(const Ptr<Decl>& decl)
{
    if (auto classDecl = DynamicCast<ClassDecl*>(decl)) {
        if (auto superClass = classDecl->GetSuperClassDecl()) {
            return superClass->GetAllSuperDecls();
        }
    }
    return {};
}

std::string GetSuperFuncCall(const Ptr<InheritableDecl>& owner, FuncDecl* funcDecl,
                             const std::vector<Ptr<ClassLikeDecl>>& canSuperCall)
{
    // if lack of function body, don't add super call
    bool isCanSuperCall =
        std::any_of(canSuperCall.begin(), canSuperCall.end(), [&owner](const Ptr<ClassLikeDecl>& decl) {
            return owner->curFile == decl->curFile && owner->begin == decl->begin && owner->end == decl->end;
        });
    if (funcDecl->TestAttr(Attribute::ABSTRACT) || (!funcDecl->TestAttr(Attribute::STATIC) && !isCanSuperCall) ||
        IsHiddenDecl(funcDecl) || IsHiddenDecl(owner)) {
        return TAB + FUNC_NOT_IMPLEMENTED_EXCEPTION + NEWLINE;
    }

    std::string superPrefix = "super.";
    if (funcDecl->TestAttr(Attribute::STATIC) && !isCanSuperCall) {
        superPrefix = owner->identifier.Val() + ".";
    }

    bool paramFirst = true;
    std::string paramsText;
    for (auto& paramList: funcDecl->funcBody->paramLists) {
        for (auto& param: paramList->params) {
            std::string paramName = param->identifier.GetRawText();
            if (keyMap.find(paramName) != keyMap.end()) {
                paramName = "`" + paramName + "`";
            }
            if (param == nullptr ||
                (param->TestAttr(Cangjie::AST::Attribute::COMPILER_ADD) &&
                 paramName == "macroCallPtr")) { continue; }
            if (paramName.empty()) {
                continue;
            }
            if (!paramFirst) {
                paramsText += ", ";
            }
            if (param->isNamedParam) {
                paramsText += paramName + ": ";
            }
            paramsText += paramName;
            paramFirst = false;
        }
    }
    return TAB + superPrefix + funcDecl->identifier + "(" + paramsText + ")" + NEWLINE;
}

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
    detail->ret = ResolveType(ty.retTy);
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
            std::unique_ptr<TypeDetail> ret = std::make_unique<CommonTypeDetail>(ty.declPtr->identifier.Val());
            return ret;
        },
        [](const GenericsTy &ty) {
            std::unique_ptr<TypeDetail> ret = std::make_unique<CommonTypeDetail>(ty.name);
            return ret;
        },
        [](const VArrayTy &ty) {
            std::unique_ptr<TypeDetail> ret = ResolveVarrayType(ty);
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
            if (!param->GetTy() || GetString(*param->GetTy()) == "UnknownType") {
                continue;
            }
            paramDetail.identifier = identifier;
            paramDetail.type = ResolveType(param->GetTy());
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
    return ResolveType(type->GetTy());
}

FuncDetail ResolveFuncDetail(const Ptr<FuncDecl>& funcDecl)
{
    FuncDetail detail;
    detail.modifiers = ResolveDeclModifiers(funcDecl);
    detail.identifier = ResolveDeclIdentifier(funcDecl);
    detail.params = ResolveFuncParamList(funcDecl);
    detail.retType = ResolveFuncRetType(funcDecl);
    detail.isOperand = OPERATOR_TO_OVERLOAD.count(funcDecl->op);
    return detail;
}

PropDetail ResolvePropDetail(const Ptr<PropDecl>& propDecl)
{
    PropDetail detail;
    detail.modifiers = ResolveDeclModifiers(propDecl);
    detail.identifier = ResolveDeclIdentifier(propDecl);
    detail.type = ResolveType(propDecl->GetTy());
    return detail;
}
} // namespace ark
