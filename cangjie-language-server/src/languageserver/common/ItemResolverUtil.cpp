// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <string>
#include "cangjie/Basic/Match.h"
#include "../logger/Logger.h"
#include "Utils.h"
#include "ItemResolverUtil.h"

using namespace Cangjie;
using namespace Cangjie::AST;

namespace ark {
std::unordered_map<std::string, int> InitKeyMap()
{
    std::unordered_map<std::string, int> keyMap = {};
    for (auto &i: TOKENS) {
        std::string token(i);
        if (token.empty() || !isalpha(token[0])) { continue; }
        keyMap[token] = 1;
    }
    return keyMap;
}

const std::unordered_map<std::string, int> keyMap = InitKeyMap();
const int ATTRIBUTE_MACRO_PARAM_NUM = MACRO_ATTR_ARGS;
const int ATTRIBUTE_MACRO_PARAM_POSITION = 0;

std::string ItemResolverUtil::ResolveNameByNode(Cangjie::AST::Node &node)
{
    if (ark::Is<MacroExpandDecl>(&node)) {
        auto decl = dynamic_cast<MacroExpandDecl *>(&node);
        if (decl->invocation.decl != nullptr) {
            return ResolveNameByNode(*decl->invocation.decl);
        }
        return "";
    } else if (ark::Is<Decl>(&node)) {
        auto decl = dynamic_cast<Decl *>(&node);
        return decl->identifier;
    } else if (ark::Is<Package>(&node)) {
        auto pkg = dynamic_cast<Package *>(&node);
        return pkg->fullPackageName;
    } else {
        return "";
    }
}

CompletionItemKind ItemResolverUtil::ResolveKindByNode(Cangjie::AST::Node &node)
{
    return Meta::match(node)(
        [](const Cangjie::AST::VarDecl & /* decl */) {
            return CompletionItemKind::CIK_VARIABLE;
        },
        [](const Cangjie::AST::FuncDecl &decl) {
            if (decl.TestAttr(Cangjie::AST::Attribute::CONSTRUCTOR)) {
                return CompletionItemKind::CIK_CONSTRUCTOR;
            }
            return CompletionItemKind::CIK_METHOD;
        },
        [](const Cangjie::AST::MacroDecl & /* decl */) {
            return CompletionItemKind::CIK_METHOD;
        },
        [](Cangjie::AST::ClassDecl & /* decl */) {
            return CompletionItemKind::CIK_CLASS;
        },
        [](Cangjie::AST::InterfaceDecl & /* decl */) {
            return ark::CompletionItemKind::CIK_INTERFACE;
        },
        [](Cangjie::AST::EnumDecl & /* decl */) {
            return CompletionItemKind::CIK_ENUM;
        },
        [](Cangjie::AST::GenericParamDecl & /* decl */) {
            return CompletionItemKind::CIK_VARIABLE;
        },
        [](Cangjie::AST::StructDecl & /* decl */) {
            return CompletionItemKind::CIK_STRUCT;
        },
        [](Cangjie::AST::TypeAliasDecl & /* decl */) {
            return CompletionItemKind::CIK_CLASS;
        },
        [](Cangjie::AST::BuiltInDecl & /* decl */) {
            return CompletionItemKind::CIK_CLASS;
        },
        [](Cangjie::AST::PrimaryCtorDecl & /* decl */) {
            return CompletionItemKind::CIK_METHOD;
        },
        [](Cangjie::AST::VarWithPatternDecl & /* decl */) {
            return CompletionItemKind::CIK_VARIABLE;
        },
        [](const Cangjie::AST::MacroExpandDecl &decl) {
            if (!decl.invocation.decl) { return CompletionItemKind::CIK_MISSING; }
            return ResolveKindByNode(*decl.invocation.decl);
        },
        // ----------- Match nothing --------------------
        []() {
            return CompletionItemKind::CIK_MISSING;
        });
}

CompletionItemKind ItemResolverUtil::ResolveKindByASTKind(Cangjie::AST::ASTKind &astKind)
{
    switch (astKind) {
        case Cangjie::AST::ASTKind::VAR_DECL:
            return CompletionItemKind::CIK_VARIABLE;
        case Cangjie::AST::ASTKind::FUNC_DECL:
            return CompletionItemKind::CIK_METHOD;
        case Cangjie::AST::ASTKind::MACRO_DECL:
            return CompletionItemKind::CIK_METHOD;
        case Cangjie::AST::ASTKind::CLASS_DECL:
            return CompletionItemKind::CIK_CLASS;
        case Cangjie::AST::ASTKind::INTERFACE_DECL:
            return ark::CompletionItemKind::CIK_INTERFACE;
        case Cangjie::AST::ASTKind::ENUM_DECL:
            return CompletionItemKind::CIK_ENUM;
        case Cangjie::AST::ASTKind::GENERIC_PARAM_DECL:
            return CompletionItemKind::CIK_VARIABLE;
        case Cangjie::AST::ASTKind::STRUCT_DECL:
            return CompletionItemKind::CIK_STRUCT;
        case Cangjie::AST::ASTKind::TYPE_ALIAS_DECL:
            return CompletionItemKind::CIK_CLASS;
        case Cangjie::AST::ASTKind::BUILTIN_DECL:
            return CompletionItemKind::CIK_CLASS;
        case Cangjie::AST::ASTKind::PRIMARY_CTOR_DECL:
            return CompletionItemKind::CIK_METHOD;
        case Cangjie::AST::ASTKind::VAR_WITH_PATTERN_DECL:
            return CompletionItemKind::CIK_VARIABLE;
        case Cangjie::AST::ASTKind::MACRO_EXPAND_DECL:
            return CompletionItemKind::CIK_METHOD;
        default:
            return CompletionItemKind::CIK_MISSING;
    }
}

std::string ItemResolverUtil::ResolveDetailByNode(Cangjie::AST::Node &node, SourceManager *sourceManager)
{
    std::string detail;
    return Meta::match(node)(
        [&detail, sourceManager](const Cangjie::AST::VarDecl &decl) {
            ResolveVarDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Cangjie::AST::FuncDecl &decl) {
            ResolveFuncDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Cangjie::AST::MacroDecl &decl) {
            ResolveMacroDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](Cangjie::AST::ClassDecl &decl) {
            ResolveClassDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](Cangjie::AST::InterfaceDecl &decl) {
            ResolveInterfaceDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Cangjie::AST::EnumDecl &decl) {
            ResolveEnumDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail](const Cangjie::AST::StructDecl &decl) {
            ResolveStructDeclDetail(detail, decl);
            return detail;
        },
        [&detail](const Cangjie::AST::GenericParamDecl &decl) {
            ResolveGenericParamDeclDetail(detail, decl);
            return detail;
        },
        [&detail, sourceManager](const Cangjie::AST::TypeAliasDecl &decl) {
            ResolveTypeAliasDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail](const Cangjie::AST::BuiltInDecl &decl) {
            ResolveBuiltInDeclDetail(detail, decl);
            return detail;
        },
        [&detail, sourceManager](const Cangjie::AST::PrimaryCtorDecl &decl) {
            ResolvePrimaryCtorDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Cangjie::AST::VarWithPatternDecl &decl) {
            ResolvePatternDetail(detail, decl.irrefutablePattern.get(), sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Cangjie::AST::MacroExpandDecl &decl) {
            if (!decl.invocation.decl) { return detail; }
            detail = ResolveDetailByNode(*decl.invocation.decl, sourceManager);
            return detail;
        },
        // ----------- Match nothing --------------------
        [&detail]() { return detail; });
}

std::string ItemResolverUtil::ResolveSignatureByNode(const Node &node, Cangjie::SourceManager *sourceManager,
                                                     bool isCompletionInsert, bool isAfterAT)
{
    std::string signature;
    return Meta::match(node)(
        [&signature](const Cangjie::AST::VarDecl &decl) {
            if (decl.TestAttr(Attribute::ENUM_CONSTRUCTOR) && decl.outerDecl) {
                return signature + decl.outerDecl->identifier + GetGenericParamByDecl(decl.outerDecl->GetGeneric()) +
                       CONSTANTS::DOT + decl.identifier;
            }
            return signature + decl.identifier;
        },
        [&signature, sourceManager, isCompletionInsert, isAfterAT](const Cangjie::AST::FuncDecl &decl) {
            ResolveFuncDeclSignature(signature, decl, sourceManager, isCompletionInsert, isAfterAT);
            // case: sth...0(), doesn't need to be shown
            return decl.TestAttr(Cangjie::AST::Attribute::HAS_INITIAL) ? "" : signature;
        },
        [&signature](const Cangjie::AST::MainDecl &decl) {
            signature += decl.identifier + "()";
            return signature;
        },
        [&signature, sourceManager](const Cangjie::AST::MacroDecl &decl) {
            ResolveMacroDeclSignature(signature, decl, sourceManager);
            return decl.TestAttr(Cangjie::AST::Attribute::HAS_INITIAL) ? "" : signature;
        },
        [&signature](const Cangjie::AST::ClassDecl &classDecl) {
            return signature + classDecl.identifier + GetGenericParamByDecl(classDecl.generic.get());
        },
        [&signature](const Cangjie::AST::InterfaceDecl &interfaceDecl) {
            return signature + interfaceDecl.identifier + GetGenericParamByDecl(interfaceDecl.generic.get());
        },
        [&signature](const Cangjie::AST::EnumDecl &enumDecl) {
            return signature + enumDecl.identifier  + GetGenericParamByDecl(enumDecl.generic.get());
        },
        [&signature](const Cangjie::AST::GenericParamDecl &genericParamDecl) {
            return signature + genericParamDecl.identifier;
        },
        [&signature](const Cangjie::AST::StructDecl &structDecl) {
            return signature + structDecl.identifier + GetGenericParamByDecl(structDecl.generic.get());
        },
        [&signature](const Cangjie::AST::Package &decl) {
            return signature + decl.fullPackageName;
        },
        [&signature](const Cangjie::AST::TypeAliasDecl &decl) {
            return signature + decl.identifier + GetGenericParamByDecl(decl.generic.get());
        },
        [&signature](const Cangjie::AST::BuiltInDecl &decl) {
            return signature + decl.identifier + GetGenericParamByDecl(decl.generic.get());
        },
        [&signature, sourceManager, isCompletionInsert, isAfterAT](const Cangjie::AST::MacroExpandDecl &decl) {
            if (!decl.invocation.decl) { return signature; }
            signature = ResolveSignatureByNode(*decl.invocation.decl, sourceManager, isCompletionInsert, isAfterAT);
            return signature;
        },
        [&signature, sourceManager](const Cangjie::AST::VarWithPatternDecl &decl) {
            ResolvePatternSignature(signature, decl.irrefutablePattern.get(), sourceManager);
            return signature;
        },
        [&signature, sourceManager, isAfterAT](const Cangjie::AST::PrimaryCtorDecl &decl) {
            ResolvePrimaryCtorDeclSignature(signature, decl, sourceManager, isAfterAT);
            return signature;
        },
        // ----------- Match nothing --------------------
        [&signature]() {
            return signature;
        });
}

std::string ItemResolverUtil::ResolveSourceByNode(Ptr<Cangjie::AST::Decl> decl, std::string path)
{
    if (!decl) {
        return "";
    }
    std::string source = "Declared in: ";

    if (decl->curFile) {
        std::string fileName = decl->curFile->fileName;

#ifdef _WIN32
        std::optional<std::string> fileNameOpt =
            Cangjie::StringConvertor::NormalizeStringToUTF8(fileName);
        if (fileNameOpt.has_value()) {
            fileName = fileNameOpt.value();
        }
        GetRealFileName(fileName, path);
#endif
        source += fileName;
    }

    source += CONSTANTS::BLANK;
    source += "Package info: ";
    source += decl->fullPackageName;

    return source;
}

std::string ItemResolverUtil::GetGenericParamByDecl(Ptr<Cangjie::AST::Generic> genericDecl)
{
    if (genericDecl == nullptr) { return ""; }
    std::string genericString = "<";
    bool first = true;
    for (auto &it:genericDecl->typeParameters) {
        if (!first) { genericString += ", "; }
        genericString += it->identifier;
        first = false;
    }
    genericString += ">";
    return genericString;
}

int ItemResolverUtil::AddGenericInsertByDecl(std::string &detail, Ptr<Cangjie::AST::Generic> genericDecl)
{
    int numParm = 0;
    if (genericDecl == nullptr) { return numParm; }
    // if Generic-T only one let it be the last one $0 because $0 can use code completion
    numParm = 1;
    detail += "<";
    bool first = true;
    for (auto &it:genericDecl->typeParameters) {
        if (!first) { detail += ", "; }
        detail += "${" + std::to_string(numParm) + ":" + it->identifier + "}";
        numParm++;
        first = false;
    }
    detail += ">";
    return numParm;
}

std::string ItemResolverUtil::GetGenericInsertByDecl(Ptr<Cangjie::AST::Generic> genericDecl)
{
    if (genericDecl == nullptr) { return ""; }
    std::string genericInsert = "<";
    bool first = true;
    // if Generic-T only one let it be the last one $0 because $0 can use code completion
    int numParm = 1;
    for (auto &it : genericDecl->typeParameters) {
        if (!it) { continue; }
        if (!first) { genericInsert += ", "; }
        genericInsert += "${" + std::to_string(numParm) + ":" + it->identifier + "}";
        numParm++;
        first = false;
    }
    genericInsert += ">";
    return genericInsert;
}

Ptr<Decl> ItemResolverUtil::GetDeclByTy(Cangjie::AST::Ty *ty)
{
    Ptr<Decl> decl = nullptr;
    if (ty == nullptr) { return nullptr; }
    return Meta::match(*ty)(
        [&](const ClassTy &classTy) {
            Ptr<Decl> decl = classTy.decl;
            return decl;
        },
        [&](const InterfaceTy &interfaceTy) {
            Ptr<Decl> decl = interfaceTy.decl;
            return decl;
        },
        [&](const StructTy &structTy) {
            Ptr<Decl> decl = structTy.decl;
            return decl;
        },
        [&](const EnumTy &enumTy) {
            Ptr<Decl> decl = enumTy.decl;
            return decl;
        },
        [&]() {
            return decl;
        });
}

std::string ItemResolverUtil::ResolveInsertByNode(const Cangjie::AST::Node &node, Cangjie::SourceManager *sourceManager,
                                                  bool isAfterAT)
{
    std::string signature;
    return Meta::match(node)(
        [&signature](const Cangjie::AST::VarDecl &varDecl) {
            return signature + varDecl.identifier;
        },
        [&signature, sourceManager, isAfterAT](const Cangjie::AST::FuncDecl &funcDecl) {
            ResolveFuncLikeDeclInsert(signature, funcDecl, sourceManager, isAfterAT);
            // case: sth...0(), doesn't need to be shown
            return funcDecl.TestAttr(Cangjie::AST::Attribute::HAS_INITIAL) ? "" : signature;
        },
        [&signature, sourceManager, isAfterAT](const Cangjie::AST::PrimaryCtorDecl &primaryCtorDecl) {
            ResolveFuncLikeDeclInsert(signature, primaryCtorDecl, sourceManager, isAfterAT);
            return signature;
        },
        [&signature, sourceManager](const Cangjie::AST::MacroDecl &macroDecl) {
            if (macroDecl.desugarDecl && macroDecl.desugarDecl->funcBody) {
                ResolveFuncLikeDeclInsert(signature, *macroDecl.desugarDecl.get(), sourceManager);
            }
            return signature.empty() ? macroDecl.identifier : signature;
        },
        [&signature](const Cangjie::AST::ClassDecl &classDecl) {
            return signature + classDecl.identifier + GetGenericInsertByDecl(classDecl.GetGeneric());
        },
        [&signature](const Cangjie::AST::InterfaceDecl &interfaceDecl) {
            return signature + interfaceDecl.identifier + GetGenericInsertByDecl(interfaceDecl.GetGeneric());
        },
        [&signature](const Cangjie::AST::EnumDecl &enumDecl) {
            return signature + enumDecl.identifier + GetGenericInsertByDecl(enumDecl.GetGeneric());
        },
        [&signature](const Cangjie::AST::GenericParamDecl &genericParamDecl) {
            return signature + genericParamDecl.identifier;
        },
        [&signature](const Cangjie::AST::StructDecl &structDecl) {
            return signature + structDecl.identifier + GetGenericInsertByDecl(structDecl.GetGeneric());
        },
        [&signature](const Cangjie::AST::Package &packageDecl) {
            return signature + packageDecl.fullPackageName;
        },
        [&signature](const Cangjie::AST::TypeAliasDecl &typeAliasDecl) {
            return signature + typeAliasDecl.identifier + GetGenericInsertByDecl(typeAliasDecl.GetGeneric());
        },
        [&signature](const Cangjie::AST::BuiltInDecl &builtInDecl) {
            return signature + builtInDecl.identifier + GetGenericInsertByDecl(builtInDecl.GetGeneric());
        },
        [&signature, sourceManager, isAfterAT](const Cangjie::AST::MacroExpandDecl &decl) {
            if (!decl.invocation.decl) { return signature; }
            signature = ResolveInsertByNode(*decl.invocation.decl, sourceManager, isAfterAT);
            return signature;
        },
        // ----------- Match nothing --------------------
        [&signature]() {
            return signature;
        });
}

std::string ItemResolverUtil::FetchTypeString(const Cangjie::AST::Type &type)
{
    if (type.ty == nullptr) {
        return "";
    }

    std::string identifier;
    return Meta::match(*(type.ty))(
        [](const ClassTy &ty) {
            return GetGenericString(ty);
        },
        [](const InterfaceTy &ty) {
            return GetGenericString(ty);
        },
        [](const EnumTy &ty) {
            return GetGenericString(ty);
        },
        [](const StructTy &ty) {
            return GetGenericString(ty);
        },
        [](const TypeAliasTy &ty) {
            return ty.declPtr->identifier.Val();
        },
        [](const GenericsTy &ty) {
            return ty.decl->identifier.Val();
        },
        [](const Ty &ty) {
            return ty.String();
        },
        [identifier]() {
            return identifier;
        });
}

template <typename T>
std::string ItemResolverUtil::GetGenericString(const T &t)
{
    std::string typeStr = t.decl->identifier;
    if (t.decl->generic) {
        typeStr += "<";
        for (const auto &parameter : t.decl->generic->typeParameters) {
            typeStr += parameter->identifier;
            typeStr += ",";
        }
        typeStr = typeStr.substr(0, typeStr.size() - 1);
        typeStr += ">";
    }
    return typeStr;
}

std::string GetModifierByNode(const Cangjie::AST::Node &node)
{
    std::string modifier;
    if (node.TestAttr(Attribute::PUBLIC)) {
        modifier += "public ";
    } else if (node.TestAttr(Attribute::PRIVATE)) {
        modifier += "private ";
    } else if (node.TestAttr(Attribute::INTERNAL)) {
        modifier += "internal ";
    } else if (node.TestAttr(Attribute::PROTECTED)) {
        modifier += "protected ";
    }

    if (node.TestAttr(Attribute::SEALED)) {
        modifier += "sealed ";
    }

    if (node.TestAttr(Attribute::STATIC)) {
        modifier += "static ";
    }

    if (node.TestAttr(Attribute::UNSAFE)) {
        modifier += "unsafe ";
    }

    if (node.TestAttr(Attribute::MUT)) {
        modifier += "mut ";
    }

    // spec: 'open' cannot modify 'abstract class'
    if (node.TestAttr(Attribute::ABSTRACT)) {
        modifier += "abstract ";
    } else if (node.TestAttr(Attribute::OPEN)) {
        modifier += "open ";
    }
    return modifier;
}

void AddTypeByNodeAndType(std::string &detail, const std::string filePath, Ptr<Cangjie::AST::Node> type,
                          Cangjie::SourceManager *sourceManager)
{
    if (type == nullptr) { return; }
    std::string result;
    if (!filePath.empty() && sourceManager) {
        int fileId = sourceManager->GetFileID(filePath);
        if (fileId >= 0) {
            result = sourceManager->GetContentBetween(static_cast<unsigned int>(fileId),
                                                      {type->GetBegin().line, type->GetBegin().column},
                                                      {type->GetEnd().line, type->GetEnd().column});
        }
    }
    try {
        if (result.empty() && !type->ToString().empty()) {
            std::string temp = type->ToString();
            Utils::TrimString(temp);
            result += temp;
        }
    } catch (NullPointerException &e) {
        Trace::Log("Invoke compiler api ToString() catch a NullPointerException");
    }
    detail += result;
}

void ItemResolverUtil::ResolveVarDeclDetail(std::string &detail, const Cangjie::AST::VarDecl &decl,
                                            SourceManager *sourceManager)
{
    if (decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR)) {
        detail = "(enum_construct) ";
        detail += ResolveSignatureByNode(decl);
        return;
    }
    detail = "(variable) ";
    detail += GetModifierByNode(decl);
    if (decl.isConst) {
        detail += "const ";
    } else if (decl.isVar) {
        detail += "var ";
    } else {
        detail += "let ";
    }
    // append identifier
    detail += ResolveSignatureByNode(decl);
    if (decl.ty != nullptr && decl.ty->kind == TypeKind::TYPE_FUNC) {
        GetDetailByTy(decl.ty, detail);
        return;
    }
    if (decl.ty == nullptr) { return; }

    // for parse info
    if (GetString(*decl.ty) == "UnknownType") {
        GetInitializerInfo(detail, decl, sourceManager, false);
        return;
    }
    std::string type = GetString(*decl.ty);
    bool hasType = false;
    if (!type.empty()) {
        detail += ": ";
        detail += GetString(*decl.ty);
        hasType = true;
    }
    GetInitializerInfo(detail, decl, sourceManager, hasType);
}

void ItemResolverUtil::GetInitializerInfo(std::string &detail, const Cangjie::AST::VarDecl &decl,
                                          SourceManager *sourceManager, bool hasType)
{
    if (decl.type != nullptr && decl.curFile != nullptr && !hasType) {
        detail += ": ";
        AddTypeByNodeAndType(detail, decl.curFile->filePath, decl.type.get(), sourceManager);
    }
    if (decl.initializer != nullptr && decl.curFile != nullptr) {
        detail += " = ";

        AddTypeByNodeAndType(detail, decl.curFile->filePath, decl.initializer.get(), sourceManager);
    }
    if (detail.length() > detailMaxLen) {
        detail = detail.substr(0, detailMaxLen);
        detail += "...";
    }
}

void ItemResolverUtil::GetDetailByTy(const Ty *ty, std::string &detail, bool isLambda)
{
    detail += "(";
    auto *ft = dynamic_cast<const FuncTy*>(ty);
    if (ft == nullptr) { return; }
    bool firstParams = true;
    for (auto &param : ft->paramTys) {
        if (!firstParams) {
            detail += ", ";
        }
        if (param != nullptr) {
            detail += GetString(*param);
        }
        firstParams = false;
    }
    detail += ")";
    if (ft->retTy != nullptr) {
        detail += isLambda ? "->" : ":";
        detail += ft->retTy->String();
    }
}

void ItemResolverUtil::ResolveFuncDeclQuickLook(std::string &quickLook, const Cangjie::AST::FuncDecl &decl,
                                                Cangjie::SourceManager *sourceManager)
{
    ResolveFuncDeclSignature(quickLook, decl, sourceManager);
    // Enhancing null-pointer judgment in release version
    if (decl.funcBody == nullptr || decl.funcBody->retType == nullptr || decl.identifier == "init") {
        return;
    }
    // append return type
    if (!FetchTypeString(*decl.funcBody->retType).empty() &&
        !decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR)
        && decl.funcBody->retType != nullptr) {
        std::string myFilePath = "";
        if (decl.outerDecl != nullptr &&
            decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
        if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
        if (myFilePath.empty()) { return; }
        quickLook += ": ";
        DealTypeDetail(quickLook, decl.funcBody->retType.get(), myFilePath, sourceManager);
    }
}

void ItemResolverUtil::ResolveMacroDeclDetail(std::string &detail, const Cangjie::AST::MacroDecl &decl,
                                              Cangjie::SourceManager *sourceManager)
{
    if (decl.desugarDecl == nullptr) {
        return;
    }
    detail = "(macro) ";
    detail += GetModifierByNode(decl);
    detail += "macro ";
    ResolveFuncDeclQuickLook(detail, *decl.desugarDecl.get(), sourceManager);
}

void ItemResolverUtil::ResolveFuncDeclDetail(std::string &detail, const Cangjie::AST::FuncDecl &decl,
                                             Cangjie::SourceManager *sourceManager)
{
    if (decl.TestAttr(Cangjie::AST::Attribute::MACRO_FUNC)) {
        detail = "(macro) ";
        detail += GetModifierByNode(decl);
        detail += "macro ";
        ResolveFuncDeclQuickLook(detail, decl, sourceManager);
    } else if (decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR)) {
        detail = "(enum_construct) ";
        ResolveFuncDeclQuickLook(detail, decl, sourceManager);
    } else {
        detail = "(function) ";
        detail += GetModifierByNode(decl);
        if (decl.propDecl) {
            detail += "prop ";
        } else {
            detail += "func ";
        }
        ResolveFuncDeclQuickLook(detail, decl, sourceManager);
        if (decl.propDecl) {
            std::string propIdentifier = decl.propDecl->identifier;
            std::string desugar = "$" + propIdentifier;
            size_t pos = detail.find(desugar);
            if (detail.find(desugar) == std::string::npos) {
                return;
            }
            detail.replace(pos, desugar.size(), propIdentifier + ".");
        }
    }
}

void ItemResolverUtil::ResolvePrimaryCtorDeclDetail(std::string &detail, const Cangjie::AST::PrimaryCtorDecl &decl,
                                                    Cangjie::SourceManager *sourceManager)
{
    detail = "(function) ";
    detail += GetModifierByNode(decl);
    detail += "func ";
    ResolvePrimaryCtorDeclSignature(detail, decl, sourceManager);

    if (decl.funcBody == nullptr || decl.funcBody->retType == nullptr) { return; }
    // append return type
    if (!FetchTypeString(*decl.funcBody->retType).empty() &&
        !decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR) && decl.funcBody->retType != nullptr) {
        std::string myFilePath = "";
        if (decl.outerDecl != nullptr &&
            decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
        if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
        if (myFilePath.empty()) { return; }
        detail += ": ";
        DealTypeDetail(detail, decl.funcBody->retType.get(), myFilePath, sourceManager);
    }
}

void ItemResolverUtil::ResolvePrimaryCtorDeclSignature(std::string &detail,
                                                       const Cangjie::AST::PrimaryCtorDecl &decl,
                                                       Cangjie::SourceManager *sourceManager, bool isAfterAT)
{
    std::string myFilePath = "";
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
    if (decl.funcBody == nullptr) { return; }
    detail += decl.identifier;
    if (decl.funcBody != nullptr && decl.funcBody->generic != nullptr &&
        !decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR)) {
        detail += GetGenericParamByDecl(decl.funcBody->generic.get());
    }
    bool isCustomAnnotationFlag = IsCustomAnnotation(decl);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "[";
    } else {
        detail += "(";
    }
    ResolveFuncParams(detail, decl.funcBody->paramLists,
                      decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR), sourceManager, myFilePath);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "]";
    } else {
        detail += ")";
    }
}

void ItemResolverUtil::ResolvePatternSignature(std::string &signature, Ptr<Cangjie::AST::Pattern> pattern,
                                               Cangjie::SourceManager *sourceManager)
{
    if (!pattern) { return; }
    if (pattern->astKind == ASTKind::ENUM_PATTERN) {
        auto pEnumPattern = dynamic_cast<EnumPattern*>(pattern.get());
        if (!pEnumPattern) { return; }
        for (auto &ptn : pEnumPattern->patterns) {
            ResolvePatternSignature(signature, ptn.get(), sourceManager);
        }
        return;
    }
    if (pattern->astKind == ASTKind::VAR_PATTERN) {
        auto pVarPattern = dynamic_cast<VarPattern*>(pattern.get());
        if (!pVarPattern || !pVarPattern->varDecl) { return; }
        signature = pVarPattern->varDecl->identifier;
    }
}

void ItemResolverUtil::ResolvePatternDetail(std::string &detail, Ptr<Cangjie::AST::Pattern> pattern,
                                            Cangjie::SourceManager *sourceManager)
{
    if (!pattern) { return; }
    if (pattern->astKind == ASTKind::ENUM_PATTERN) {
        auto pEnumPattern = dynamic_cast<EnumPattern*>(pattern.get());
        if (!pEnumPattern) { return; }
        for (auto &ptn : pEnumPattern->patterns) {
            ResolvePatternDetail(detail, ptn.get(), sourceManager);
        }
        return;
    }
    if (pattern->astKind == ASTKind::VAR_PATTERN) {
        auto pVarPattern = dynamic_cast<VarPattern*>(pattern.get());
        if (!pVarPattern || !pVarPattern->varDecl) { return; }
        ResolveVarDeclDetail(detail, *pVarPattern->varDecl.get(), sourceManager);
    }
}

void ItemResolverUtil::ResolveFuncParams(std::string &detail,
                                         const std::vector<OwnedPtr<FuncParamList>> &paramLists,
                                         bool isEnumConstruct,
                                         Cangjie::SourceManager *sourceManager,
                                         const std::string &filePath)
{
    // append params
    bool firstParams = true;
    for (auto &paramList : paramLists) {
        for (auto &param : paramList->params) {
            std::string paramName = param->identifier.GetRawText();
            if (keyMap.find(paramName)!= keyMap.end()) {
                paramName = "`" + paramName + "`";
            }
            if (param == nullptr ||
                (param->TestAttr(Cangjie::AST::Attribute::COMPILER_ADD) &&
                 paramName == "macroCallPtr")) { return; }
            if (!firstParams) {
                detail += ", ";
            }
            firstParams = false;
            if (!paramName.empty() && !isEnumConstruct) {
                detail += paramName;
                detail += param->isNamedParam ? "!: " : ": ";
            }
            if (param->ty == nullptr) {
                continue;
            }
            bool getTypeByNodeAndType = param->type != nullptr &&
                (GetString(*param->ty) == "UnknownType" ||
                (sourceManager && param->type->astKind == Cangjie::AST::ASTKind::FUNC_TYPE));
            if (getTypeByNodeAndType) {
                AddTypeByNodeAndType(detail, filePath, param->type.get(), sourceManager);
            } else {
                detail += GetString(*param->ty);
            }
            try {
                if (param->assignment != nullptr && param->assignment.get() &&
                    !param->assignment.get()->ToString().empty()) {
                    detail += " = ";
                    AddTypeByNodeAndType(detail, filePath, param->assignment.get(), sourceManager);
                }
            } catch (NullPointerException &e) {
                Trace::Log("Invoke compiler api catch a NullPointerException");
            }
        }
        size_t variadicIndex = paramList->variadicArgIndex;
        if (variadicIndex > 0) {
            detail += ", ...";
        }
    }
}

void ItemResolverUtil::ResolveMacroParams(std::string &detail,
                                          const std::vector<OwnedPtr<FuncParamList>> &paramLists)
{
    for (auto &paramList : paramLists) {
        if (paramList && paramList->params.size() == ATTRIBUTE_MACRO_PARAM_NUM &&
            paramList->params[ATTRIBUTE_MACRO_PARAM_POSITION]) {
            detail = detail + "[" + paramList->params[ATTRIBUTE_MACRO_PARAM_POSITION]->identifier + ": Tokens]";
        }
    }
}

void ItemResolverUtil::ResolveMacroParamsInsert(std::string &detail,
                                                const std::vector<OwnedPtr<FuncParamList>> &paramLists)
{
    for (auto &paramList : paramLists) {
        if (paramList && paramList->params.size() == ATTRIBUTE_MACRO_PARAM_NUM &&
            paramList->params[ATTRIBUTE_MACRO_PARAM_POSITION]) {
            detail = detail + "[${1:" + paramList->params[ATTRIBUTE_MACRO_PARAM_POSITION]->identifier + ": Tokens}]";
        }
    }
}

void ItemResolverUtil::ResolveFuncDeclSignature(std::string &detail, const Cangjie::AST::FuncDecl &decl,
                                                Cangjie::SourceManager *sourceManager, bool isCompletionInsert,
                                                bool isAfterAT)
{
    std::string myFilePath = "";
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
    if (decl.funcBody == nullptr) { return; }
    if (decl.identifier.Val() == "arrayInit1" || decl.identifier.Val() == "arrayInit2" &&
        decl.fullPackageName == "std.core") {
        return;
    }
    if (decl.TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
        if (decl.outerDecl) {
            detail += decl.outerDecl->identifier + GetGenericParamByDecl(decl.outerDecl->GetGeneric()) + ".";
        }
    }
    detail += decl.identifier;
    if (isCompletionInsert && decl.TestAttr(Cangjie::AST::Attribute::MACRO_FUNC)) {
        ResolveMacroParams(detail, decl.funcBody->paramLists);
        return;
    }
    if (decl.funcBody->generic != nullptr && !decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR)) {
        detail += GetGenericParamByDecl(decl.funcBody->generic.get());
    }
    bool isCustomAnnotationFlag = IsCustomAnnotation(decl);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "[";
    } else {
        detail += "(";
    }
    ResolveFuncParams(detail, decl.funcBody->paramLists,
                      decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR),
                      sourceManager, myFilePath);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "]";
    } else {
        detail += ")";
    }
}

void ItemResolverUtil::ResolveMacroDeclSignature(std::string &detail, const Cangjie::AST::MacroDecl &decl,
                                                 Cangjie::SourceManager *sourceManager)
{
    if (decl.desugarDecl == nullptr || decl.desugarDecl->funcBody == nullptr || decl.curFile == nullptr) {
        return;
    }
    detail += decl.identifier;
    if (sourceManager && decl.desugarDecl && decl.desugarDecl.get()->funcBody) {
        ResolveMacroParams(detail, decl.desugarDecl.get()->funcBody->paramLists);
        return;
    }
    detail += "(";
    ResolveFuncParams(detail, decl.desugarDecl->funcBody->paramLists, false, sourceManager,
                      decl.curFile->filePath);
    detail += ")";
}

int ResolveFuncParamInsert(std::string &detail, const std::string myFilePath, Ptr<FuncParam> param, int numParm,
                           Cangjie::SourceManager *sourceManager, bool isEnumConstruct)
{
    std::string paramName = isEnumConstruct ? "": param->identifier.GetRawText();
    if (keyMap.find(paramName)!= keyMap.end()) {
        paramName = "`" + paramName + "`";
    }
    if (param->isNamedParam) {
        detail += (paramName.empty() ? "" : (paramName + ": "));
        detail += "${" + std::to_string(numParm) + ":";
        numParm++;
        bool flag = param->ty && param->assignment && param->assignment.get()
                    && !param->assignment.get()->ToString().empty();
        if (GetString(*param->ty) == "UnknownType" && param->type != nullptr) {
            AddTypeByNodeAndType(detail, myFilePath, param->type.get(), sourceManager);
            detail += flag ? " = " : "";
            AddTypeByNodeAndType(detail, myFilePath, param->assignment.get(), sourceManager);
        } else {
            detail += param->ty ? GetString(*param->ty) : "";
            detail += flag ? (" = " + param->assignment.get()->ToString()) : "";
        }
    } else {
        detail += "${" + std::to_string(numParm) + ":";
        numParm++;
        detail += (paramName.empty() ? "" : (paramName + ": "));
        if (GetString(*param->ty) == "UnknownType" && param->type != nullptr) {
            AddTypeByNodeAndType(detail, myFilePath, param->type.get(), sourceManager);
        } else {
            detail += (param->ty ? GetString(*param->ty) : "");
        }
    }
    return numParm;
}

template<typename T>
void ItemResolverUtil::ResolveFuncLikeDeclInsert(std::string &detail, const T &decl,
                                                 Cangjie::SourceManager *sourceManager, bool isAfterAT)
{
    std::string myFilePath = "";
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
    detail += decl.identifier;
    if (decl.TestAttr(Cangjie::AST::Attribute::MACRO_FUNC)) {
        ResolveMacroParamsInsert(detail, decl.funcBody->paramLists);
        return;
    }
    int numParm = 1;
    bool isEnumConstruct = false;
    if (!decl.TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR)) {
        numParm = AddGenericInsertByDecl(detail, decl.GetGeneric()) + 1;
        if ((decl.identifier.Val().compare("init") == 0 && decl.astKind == ASTKind::FUNC_DECL ||
             decl.astKind == ASTKind::PRIMARY_CTOR_DECL) && decl.outerDecl != nullptr) {
            std::string temp = "";
            numParm = AddGenericInsertByDecl(temp, decl.outerDecl->GetGeneric()) + 1;
        }
    } else {
        isEnumConstruct = true;
    }
    bool isCustomAnnotationFlag = IsCustomAnnotation(decl);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "[";
    } else {
        detail += "(";
    }
    // append params
    bool firstParams = true;
    if (decl.funcBody == nullptr || decl.funcBody->paramLists.empty()) { return; }
    auto paramList = decl.funcBody->paramLists[0].get();
    for (auto &param : paramList->params) {
        if (!firstParams) {
            detail += ", ";
        }
        if (param) {
            numParm = ResolveFuncParamInsert(detail, myFilePath, param.get(), numParm, sourceManager, isEnumConstruct);
        }
        firstParams = false;
        detail += "}";
    }
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "]";
    } else {
        detail += ")";
    }
}

void ItemResolverUtil::ResolveClassDeclDetail(std::string &detail, Cangjie::AST::ClassDecl &decl,
                                              Cangjie::SourceManager *sourceManager)
{
    detail = "(class) ";
    detail += GetModifierByNode(decl);
    detail += "class ";
    detail += ResolveSignatureByNode(decl);
    // Do not display basic type -- Object.
    if (decl.inheritedTypes.size() == 0) {
        return;
    }
    if (decl.inheritedTypes.size() == 1 && decl.inheritedTypes[0]->ty->IsObject()) {
        return;
    }
    detail += " <: ";
    bool isFirst = true;
    for (auto &iterFace : decl.inheritedTypes) {
        if (iterFace == nullptr || decl.curFile == nullptr || iterFace->ty->IsObject()) {
            continue;
        }
        if (!isFirst) {
            detail += " & ";
        }
        DealTypeDetail(detail, iterFace.get(), decl.curFile->filePath, sourceManager);
        isFirst = false;
    }
}

void ItemResolverUtil::ResolveInterfaceDeclDetail(std::string &detail, Cangjie::AST::InterfaceDecl &decl,
                                                  Cangjie::SourceManager *sourceManager)
{
    detail = "(interface) ";
    detail += GetModifierByNode(decl);
    detail += "interface ";
    detail += ResolveSignatureByNode(decl);
    if (decl.inheritedTypes.empty()) {
        return;
    }
    detail += " extends ";
    bool firstInterface = true;
    for (auto &iterFace : decl.inheritedTypes) {
        if (iterFace == nullptr || decl.curFile == nullptr) { continue; }
        if (!firstInterface) {
            detail += ", ";
        }
        DealTypeDetail(detail, iterFace.get(), decl.curFile->filePath, sourceManager);
        firstInterface = false;
    }
}

void ItemResolverUtil::ResolveEnumDeclDetail(std::string &detail, const Cangjie::AST::EnumDecl &decl,
                                             Cangjie::SourceManager *sourceManager)
{
    detail = "(enum) ";
    detail += GetModifierByNode(decl);
    detail += "enum ";
    detail += ResolveSignatureByNode(decl);
    if (decl.inheritedTypes.size() == 0) {
        return;
    }
    detail += " <: ";
    bool isFirst = true;
    for (auto &type : decl.inheritedTypes) {
        if (type == nullptr || decl.curFile == nullptr) { continue; }
        if (!isFirst) {
            detail += " & ";
        }
        DealTypeDetail(detail, type.get(), decl.curFile->filePath, sourceManager);
        isFirst = false;
    }
}

void ItemResolverUtil::ResolveStructDeclDetail(std::string &detail, const Cangjie::AST::StructDecl &decl)
{
    detail = "(struct) ";
    detail += GetModifierByNode(decl);
    detail += "struct ";
    detail += ResolveSignatureByNode(decl);
}

void ItemResolverUtil::ResolveGenericParamDeclDetail(std::string &detail, const Cangjie::AST::GenericParamDecl &decl)
{
    detail += "genericParam ";
    detail += ResolveSignatureByNode(decl);
}

void ItemResolverUtil::ResolveTypeAliasDetail(std::string &detail, const Cangjie::AST::TypeAliasDecl &decl,
                                              Cangjie::SourceManager *sourceManager)
{
    detail += GetModifierByNode(decl);
    detail += "type ";
    detail += ResolveSignatureByNode(decl);
    if (decl.type == nullptr) {
        return;
    }
    detail += " = ";
    if (decl.curFile != nullptr && FetchTypeString(*decl.type.get()) == "UnknownType") {
        AddTypeByNodeAndType(detail, decl.curFile->filePath, decl.type.get(), sourceManager);
        return;
    }

    detail += FetchTypeString(*decl.type);
}

void ItemResolverUtil::DealTypeDetail(std::string &detail, Ptr<Cangjie::AST::Type> type,
                                      const std::string &filePath, Cangjie::SourceManager *sourceManager)
{
    if (!type) { return; }
    if (FetchTypeString(*type) == "UnknownType") {
        AddTypeByNodeAndType(detail, filePath, type, sourceManager);
        return;
    }
    detail += FetchTypeString(*type);
}

void ItemResolverUtil::ResolveBuiltInDeclDetail(std::string &detail, const BuiltInDecl &decl)
{
    detail = "(BuiltInType) ";
    detail += GetModifierByNode(decl);
    detail += "Type ";
    detail += ResolveSignatureByNode(decl);
}

bool ItemResolverUtil::IsCustomAnnotation(const Cangjie::AST::Decl &decl)
{
    if (!decl.outerDecl) {
        return false;
    }
    auto outerDecl = decl.outerDecl;
    for (auto &annotation : outerDecl->annotations) {
        if (!annotation) { continue; }
        if (annotation->astKind == ASTKind::ANNOTATION) { return true; }
    }
    return false;
}
} // namespace ark
