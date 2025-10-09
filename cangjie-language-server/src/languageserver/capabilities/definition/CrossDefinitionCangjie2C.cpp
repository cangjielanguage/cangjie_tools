// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CrossDefinitionCangjie2C.h"

using namespace std;
using namespace Cangjie;
using namespace Cangjie::AST;

namespace ark {
std::unordered_map<TypeKind, std::string> CANGJIE2C = {{TypeKind::TYPE_UNIT,        "void"},
                                                       {TypeKind::TYPE_BOOLEAN,     "bool"},
                                                       {TypeKind::TYPE_INT8,        "int8_t"},
                                                       {TypeKind::TYPE_INT16,       "int16_t"},
                                                       {TypeKind::TYPE_INT32,       "int32_t"},
                                                       {TypeKind::TYPE_INT64,       "int64_t"},
                                                       {TypeKind::TYPE_UINT8,       "uint8_t"},
                                                       {TypeKind::TYPE_UINT16,      "uint16_t"},
                                                       {TypeKind::TYPE_UINT32,      "uint32_t"},
                                                       {TypeKind::TYPE_UINT64,      "uint64_t"},
                                                       {TypeKind::TYPE_INT_NATIVE,  "ssize_t"},
                                                       {TypeKind::TYPE_UINT_NATIVE, "size_t"},
                                                       {TypeKind::TYPE_FLOAT32,     "float"},
                                                       {TypeKind::TYPE_FLOAT64,     "double"},
                                                       {TypeKind::TYPE_CSTRING,     "const char *"},
                                                       {TypeKind::TYPE_POINTER,     ""},
                                                       {TypeKind::TYPE_STRUCT,      ""},
                                                       {TypeKind::TYPE_VARRAY,      ""}};

void ConvertFirstVArray(std::string& str)
{
    if ((str.find_last_of(']') - str.find_first_of('[')) != 1) {
        (void) str.replace(str.find_first_of('['), string("[]").length(), "(*)");
    } else {
        (void) str.replace(str.find_first_of('['), string("[]").length(), "*");
    }
}

std::string CrossDefinitionCangjie2C::TypeVarray(const Cangjie::AST::Ty *ty, bool isSimple)
{
    for (auto &item: ty->typeArgs) {
        if (!isSimple) {
            if (item->kind == TypeKind::TYPE_VARRAY || item->kind == TypeKind::TYPE_CSTRING) {
                return GetCType(item) + "[]";
            }
            if (item->kind == TypeKind::TYPE_POINTER) {
                const string baseType = GetCType(item, true);
                return baseType == CANGJIE2C.find(TypeKind::TYPE_CSTRING)->second ? baseType + "*[]" :
                       baseType + " *[]";
            }
            return GetCType(item) + " []";
        }
        return GetCType(item, true);
    }
    return "unknown";
}

std::string CrossDefinitionCangjie2C::TypeCpointer(const Cangjie::AST::Ty *ty, bool isSimple)
{
    for (auto &item: ty->typeArgs) {
        if (!isSimple) {
            if (item->kind == TypeKind::TYPE_POINTER || item->kind == TypeKind::TYPE_CSTRING) {
                return GetCType(item) + "*";
            }
            if (item->kind == TypeKind::TYPE_VARRAY) {
                const string baseType = GetCType(item, true);
                return baseType == CANGJIE2C.find(TypeKind::TYPE_CSTRING)->second ? baseType + "*" :
                       baseType + " *";
            }
            return GetCType(item, true) + " *";
        }
        return GetCType(item, true);
    }
    return "unknown";
}

void CrossDefinitionCangjie2C::Cangjie2CGetFuncMessage(std::vector<message> &CrossMessage,
                                                       Ptr<Cangjie::AST::FuncDecl> funcDecl)
{
    message message;
    bool inValid = !funcDecl || !funcDecl->funcBody || !funcDecl->funcBody->retType ||
                   !funcDecl->funcBody->retType->ty || funcDecl->funcBody->paramLists.empty() ||
                   GetCType(funcDecl->funcBody->retType->ty) == "unknown";
    if (inValid) { return; }
    for (const auto &item: funcDecl->funcBody->paramLists[0]->params) {
        string basicString = GetCType(item->ty);
        if (basicString == "unknown") {
            message.functionParameters = {};
            return;
        }
        // first [] to *
        if (item->ty->kind == TypeKind::TYPE_VARRAY) {
            ConvertFirstVArray(basicString);
        }
        (void) message.functionParameters.emplace_back(basicString);
    }
    message.targetLanguage = "C";
    message.functionName = funcDecl->identifier;
    message.retType = GetCType(funcDecl->funcBody->retType->ty);
    if (funcDecl->hasVariableLenArg) {
        (void) message.functionParameters.emplace_back("...");
    }
    (void) CrossMessage.emplace_back(message);
}

std::string CrossDefinitionCangjie2C::GetCType(const Cangjie::AST::Ty *ty, bool isSimple)
{
    if (ty && CANGJIE2C.find(ty->kind) != CANGJIE2C.end()) {
        if (ty->kind == TypeKind::TYPE_STRUCT) {
            return ty->name;
        } else if (ty->kind == TypeKind::TYPE_POINTER) {
            return TypeCpointer(ty, isSimple);
        } else if (ty->kind == TypeKind::TYPE_VARRAY) {
            return TypeVarray(ty, isSimple);
        }
        return CANGJIE2C.find(ty->kind)->second;
    }
    return "unknown";
}
}
