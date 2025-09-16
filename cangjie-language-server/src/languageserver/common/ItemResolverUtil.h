// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_ITEMRESOLVERUTIL_H
#define LSPSERVER_ITEMRESOLVERUTIL_H

#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Types.h"
#include "../../json-rpc/CompletionType.h"
#include "../../json-rpc/Protocol.h"
#include "cangjie/AST/Node.h"
#include "cangjie/Basic/SourceManager.h"

namespace ark {
std::unordered_map<std::string, int> InitKeyMap();

class ItemResolverUtil {
public:
    static std::string ResolveNameByNode(Cangjie::AST::Node &node);

    static CompletionItemKind ResolveKindByNode(Cangjie::AST::Node &node);

    static CompletionItemKind ResolveKindByASTKind(Cangjie::AST::ASTKind &astKind);

    static std::string ResolveDetailByNode(Cangjie::AST::Node &node, Cangjie::SourceManager *sourceManager = nullptr);

    static std::string ResolveSourceByNode(Ptr<Cangjie::AST::Decl> decl, std::string path);

    static std::string ResolveSignatureByNode(const Cangjie::AST::Node &node,
                                              Cangjie::SourceManager *sourceManager = nullptr,
                                              bool isCompletionInsert = false,
                                              bool isAfterAT = false);

    static std::string ResolveInsertByNode(const Cangjie::AST::Node &primaryCtorDecl,
                                           Cangjie::SourceManager *sourceManager = nullptr, bool isAfterAT = false);

    static Ptr<Cangjie::AST::Decl> GetDeclByTy(Cangjie::AST::Ty *enumTy);

    static std::string GetGenericInsertByDecl(Ptr<Cangjie::AST::Generic> genericDecl);

    static void ResolveFuncParams(std::string &detail,
                                  const std::vector<OwnedPtr<Cangjie::AST::FuncParamList>> &paramLists,
                                  bool isEnumConstruct = false, Cangjie::SourceManager *sourceManager = nullptr,
                                  const std::string &filePath = "",
                                  bool needLastParam = true);

    static void ResolveMacroParams(std::string &detail,
        const std::vector<OwnedPtr<Cangjie::AST::FuncParamList>> &paramLists);

    static void GetDetailByTy(const Cangjie::AST::Ty *ty, std::string &detail, bool isLambda = false);

    static std::string GetGenericParamByDecl(Ptr<Cangjie::AST::Generic> genericDecl);

    static bool IsCustomAnnotation(const Cangjie::AST::Decl &decl);

    static void AddTypeByNodeAndType(std::string &detail, const std::string filePath, Ptr<Cangjie::AST::Node> type,
        Cangjie::SourceManager *sourceManager);

    static std::string FetchTypeString(const Cangjie::AST::Type &type);

    static void DealTypeDetail(std::string &detail, Ptr<Cangjie::AST::Type> type, const std::string &filePath,
        Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveFuncTypeParamSignature(std::string &detail,
        const std::vector<OwnedPtr<Cangjie::AST::Type>> &paramTypes,
        Cangjie::SourceManager *sourceManager, const std::string &filePath, bool needLastParam = true);

    static void ResolveFuncTypeParamInsert(std::string &detail,
        const std::vector<OwnedPtr<Cangjie::AST::Type>> &paramTypes, Cangjie::SourceManager *sourceManager,
        const std::string &filePath, int &numParm, bool needLastParam = true, bool needDefaultParamName = false);

    static std::string ResolveFollowLambdaSignature(const Cangjie::AST::Node &node,
        Cangjie::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static std::string ResolveFollowLambdaInsert(const Cangjie::AST::Node &node,
        Cangjie::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static void ResolveParamListFuncTypeVarDecl(const Cangjie::AST::Node &node, std::string &label,
        std::string &insertText, Cangjie::SourceManager *sourceManager = nullptr);

    static int AddGenericInsertByDecl(std::string &detail, Ptr<Cangjie::AST::Generic> genericDecl);

    static int ResolveFuncParamInsert(std::string &detail, const std::string myFilePath,
        Ptr<Cangjie::AST::FuncParam> param, int numParm, Cangjie::SourceManager *sourceManager, bool isEnumConstruct);

private:

    template <typename T>
    static std::string GetGenericString(const T &t);

    static void GetInitializerInfo(std::string &detail, const Cangjie::AST::VarDecl &decl,
                                   Cangjie::SourceManager *sourceManager, bool hasType);

    static void ResolveVarDeclDetail(std::string &detail, const Cangjie::AST::VarDecl &decl,
                                     Cangjie::SourceManager *sourceManager = nullptr);
    
    static void ResolveFuncDeclQuickLook(std::string &detail, const Cangjie::AST::FuncDecl &decl,
                                         Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveFuncDeclDetail(std::string &detail, const Cangjie::AST::FuncDecl &decl,
                                      Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolvePrimaryCtorDeclDetail(std::string &detail, const Cangjie::AST::PrimaryCtorDecl &decl,
                                             Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolvePrimaryCtorDeclSignature(std::string &detail, const Cangjie::AST::PrimaryCtorDecl &decl,
                                                Cangjie::SourceManager *sourceManager = nullptr,
                                                bool isAfterAT = false);

    static void ResolvePatternSignature(std::string &signature, Ptr<Cangjie::AST::Pattern> pattern,
                                        Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolvePatternDetail(std::string &detail, Ptr<Cangjie::AST::Pattern> pattern,
                                     Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveMacroDeclDetail(std::string &detail, const Cangjie::AST::MacroDecl &decl,
                                       Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveMacroDeclSignature(std::string &detail, const Cangjie::AST::MacroDecl &decl,
                                          Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveClassDeclDetail(std::string &detail, Cangjie::AST::ClassDecl &decl,
                                       Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveInterfaceDeclDetail(std::string &detail, Cangjie::AST::InterfaceDecl &decl,
                                           Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveEnumDeclDetail(std::string &detail, const Cangjie::AST::EnumDecl &decl,
                                      Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveFuncDeclSignature(std::string &detail, const Cangjie::AST::FuncDecl &decl,
                                         Cangjie::SourceManager *sourceManager = nullptr,
                                         bool isCompletionInsert = false,
                                         bool isAfterAT = false);

    template<typename T>
    static void ResolveFuncLikeDeclInsert(std::string &detail,
                                          const T &decl,
                                          Cangjie::SourceManager *sourceManager = nullptr,
                                          bool isAfterAT = false);

    static void ResolveStructDeclDetail(std::string &detail, const Cangjie::AST::StructDecl &decl);

    static void ResolveGenericParamDeclDetail(std::string &detail, const Cangjie::AST::GenericParamDecl &decl);

    static void ResolveTypeAliasDetail(std::string &detail, const Cangjie::AST::TypeAliasDecl &decl,
                                       Cangjie::SourceManager *sourceManager = nullptr);

    static void ResolveBuiltInDeclDetail(std::string &detail, const Cangjie::AST::BuiltInDecl &decl);
 
    static void ResolveMacroParamsInsert(std::string &detail,
                                         const std::vector<OwnedPtr<Cangjie::AST::FuncParamList>> &paramLists);

    template<typename T> static void ResolveFollowLambdaFuncSignature(std::string &detail, const T &decl,
        Cangjie::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static void ResolveFollowLambdaVarSignature(std::string &detail, const Cangjie::AST::VarDecl &decl,
        Cangjie::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    template<typename T> static void ResolveFollowLambdaFuncInsert(std::string &detail, const T &decl,
        Cangjie::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static void ResolveFollowLambdaVarInsert(std::string &detail, const Cangjie::AST::VarDecl &decl,
        Cangjie::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static const int detailMaxLen = 256;

    template<typename T> static int BuildLambdaFuncPreParamInsert(const T &decl, Cangjie::SourceManager *sourceManager,
        OwnedPtr<Cangjie::AST::FuncParamList> &paramList, const std::string &myFilePath, std::string &insertText);

    static void GetFuncNamedParam(std::string &detail, Cangjie::SourceManager *sourceManager,
        const std::string &filePath, const OwnedPtr<Cangjie::AST::FuncParam> &param);

    template<typename T> static void DealEmptyParamFollowLambda(const T &decl,
        Cangjie::SourceManager *sourceManager, OwnedPtr<Cangjie::AST::FuncParamList> &paramList, std::string &signature,
        const std::string &myFilePath);
};
} // namespace ark

#endif // LSPSERVER_ITEMRESOLVERUTIL_H
