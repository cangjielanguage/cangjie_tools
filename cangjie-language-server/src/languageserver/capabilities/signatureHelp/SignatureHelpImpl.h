// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_SIGNATUREHELP_H
#define LSPSERVER_SIGNATUREHELP_H
#include "../../common/Callbacks.h"
#include "../../logger/Logger.h"
#include "cangjie/Lex/Token.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Symbol.h"
#include "cangjie/Basic/Match.h"
#include "cangjie/Utils/FileUtil.h"
#include "cangjie/Modules/ImportManager.h"
#include "../../ArkAST.h"

namespace ark {
class SignatureHelpImpl {
public:
    SignatureHelpImpl(const ArkAST &ast,
                      SignatureHelp &result,
                      Cangjie::ImportManager &importManager,
                      const SignatureHelpParams &params,
                      const Cangjie::Position &pos);

    ~SignatureHelpImpl() = default;

    void FindSignatureHelp();

private:
    void SetRealTokensAndIndex();

    void NormalFuncSignatureHelp();

    bool MemberFuncSignatureHelp();

    void FindFunDeclByType(Cangjie::AST::Ty &nodeTy, const std::string funcName);

    void FindFunDeclByNode(Cangjie::AST::Node &node);

    void FindFuncDeclByDeclType(Ptr<Cangjie::AST::Ty> declTy, const std::string& funcName);

    void FillingDeclsInPackage(std::string &packageName, const std::string &funcName,
                               const Cangjie::AST::Node &curNode);

    void ResolveFuncDecl(Cangjie::AST::Decl &decl);

    void ResolveClassDecl(Cangjie::AST::Node &node);

    int CalActiveParaAndParamPos();

    void CalBackParamPos(const int &index);

    void DealRetrigger();

    void FindRealActiveParamPos();

    bool checkModifier(const std::string curPkg, Ptr<const Cangjie::AST::Decl> decl) const;

    std::string ResolveFuncName();

    void ResolveParameter(std::string &detail, bool &firstParams, const OwnedPtr<FuncParam> &paramPtr,
                          Signatures &signatures);

    bool IsFuncDeclValid(Ptr<Cangjie::AST::FuncDecl> funcDecl);

    void FindSuperClassInit(const std::vector<Symbol*>& query);

    bool checkAccess(const std::string curPkg, const Cangjie::AST::Decl &decl) const;

    int GetDotIndex() const;

    int GetFuncNameIndex() const;

    const ArkAST *ast = nullptr;
    std::pair<std::vector<Token>, int> realTokensAndIndex = {{}, -1};
    SignatureHelp *result = {};
    Cangjie::ImportManager *importManager = nullptr;
    SignatureHelpParams params = {};
    Cangjie::Position pos = {};
    std::set<std::string> signatureLabel = {};
    std::string packageNameForPath;
    bool hasNameParam = false;
    int nameParamPos = -1;
    int leftQuoteIndex = -1;
    int funcNameIndex = -1;
    unsigned int offset = 0;
    std::set<Position> visitedFunc = {};
    const int meanNoMatchParameter = 500;
    std::string fatherCLassName;
    bool isThis = false;
};
} // namespace ark

#endif // LSPSERVER_SIGNATUREHELP_H
