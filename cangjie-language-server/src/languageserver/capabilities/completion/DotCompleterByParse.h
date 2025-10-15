// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_DOTCOMPLETERBYPARSE_H
#define LSPSERVER_DOTCOMPLETERBYPARSE_H

#include <cangjie/AST/ASTContext.h>
#include <cangjie/AST/Node.h>
#include <cangjie/AST/ScopeManagerApi.h>
#include <cangjie/AST/Symbol.h>
#include <cstdint>
#include "../../CompilerCangjieProject.h"
#include "../../common/ItemResolverUtil.h"
#include "CompletionEnv.h"
#include "CompletionImpl.h"
#include "cangjie/AST/Walker.h"

namespace ark {
class DotCompleterByParse {
public:
    DotCompleterByParse(Cangjie::ASTContext &ctx, CompletionResult &res, Cangjie::ImportManager &importManager)
        : context(&ctx), result(res), importManager(&importManager)
    {
        if (context && context->curPackage && !context->curPackage->files.empty() && context->curPackage->files[0]) {
            curFilePath = context->curPackage->files[0]->filePath;
            packageNameForPath = GetPkgNameFromNode(context->curPackage->files[0].get());
            auto moduleName = Utils::SplitQualifiedName(packageNameForPath).front();
            syscap.SetIntersectionSet(moduleName);
        }
        InitMap();
    }

    ~DotCompleterByParse()= default;

    void Complete(const ArkAST &input, const Cangjie::Position &pos, const std::string &prefix = "");

    void CompleteFromType(const std::string &identifier,
                          const Cangjie::Position &pos,
                          Cangjie::AST::Ty *type,
                          CompletionEnv &env) const;

private:
    void CompleteClassDecl(Ptr<Cangjie::AST::Ty> ty,
                           const Cangjie::Position &pos,
                           CompletionEnv &env,
                           bool isSuperOrThis = false) const;

    void CompleteInterfaceDecl(Ptr<const Cangjie::AST::InterfaceDecl> interfaceDecl,
                               const Cangjie::Position &pos,
                               CompletionEnv &env) const;

    void CompleteEnumDecl(Ptr<Cangjie::AST::Ty> ty, const Cangjie::Position &pos, CompletionEnv &env) const;

    void CompleteStructDecl(Ptr<Cangjie::AST::Ty> ty, const Cangjie::Position &pos, CompletionEnv &env) const;

    void CompleteBuiltInType(Ty *type, CompletionEnv &env) const;

    void FillingDeclsInPackageDot(std::pair<std::string, CompletionEnv &> pkgNameAndEnv,
                                  Cangjie::AST::Node &curNode,
                                  std::vector<OwnedPtr<Cangjie::AST::File>> &files,
                                  std::set<std::string> lastDeclSet = {"*"},
                                  const std::pair<bool, bool> openFillAndIsImport = {false, false});

    std::set<std::string> FindDeclSetByFiles(const std::string &packageName,
                                             std::vector<OwnedPtr<Cangjie::AST::File>> &files) const;

    void AddExtendDeclFromIndex(Ptr<Ty> &extendTy, CompletionEnv &env, const Position &pos) const;

    std::string QueryByPos(Ptr<Node> node, const Position pos);

    static bool Contain(Ptr<Node> node, const Position pos)
    {
        return node && node->begin.fileID == pos.fileID && node->begin <= pos && pos < node->end;
    };

    bool CheckHasLocalDecl(const std::string &beforePrefix, const std::string &scopeName,
                           Ptr<Expr> expr, const Position &pos) const;

    void DeepFind(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void InitMap() const;

    void FindFuncDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindFuncBody(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindBlock(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindVarDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindVarWithPatternDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindClassDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindStructDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindInterfaceDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindEnumDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMainDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindExtendDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindIfExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindWhileExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindDoWhileExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindForInExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindLambdaExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMatchExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindVarPattern(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindBinaryExpr(Ptr<Node> node, const Position &pos, std::string & /* scopeName */, bool &isInclude);

    void FindOptionalChainExpr(Ptr<Node> node, const Position &pos, std::string & /* scopeName */, bool &isInclude);

    void FindAssignExpr(Ptr<Node> node, const Position &pos, std::string & /* scopeName */, bool &isInclude);

    void FindIncOrDecExpr(Ptr<Node> node, const Position &pos, std::string & /* scopeName */, bool &isInclude);

    void FindMemberAccess(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindRefExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindCallExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindPrimaryCtorDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindReturnExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindTryExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMacroDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMacroExpandDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMacroExpandExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindSynchronizedExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindPropDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindSpawnExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindTrailingClosureExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void CompleteQualifiedType(const std::string &beforePrefix, CompletionEnv &env) const;

    void FuzzyDotComplete(const ArkAST &input, const Position &pos, const std::string &prefix, CompletionEnv &env);

    void CompleteSuperInterface(Ptr<const InterfaceDecl> interfaceDecl, const Position &pos, CompletionEnv &env) const;

    void CompleteEnumInterface(Ptr<EnumDecl> enumDecl, const Position &pos, CompletionEnv &env) const;

    bool IsEnumCtorTy(const std::string &beforePrefix, Ty *const &ty) const;

    Ptr<Decl> FindTopDecl(const ArkAST &input, const std::string &prefix, CompletionEnv &env,
                          const Position &pos);

    void CompleteCandidate(const Position &pos, const std::string &prefix, CompletionEnv &env,
                           Candidate &declOrTy);

    Position GetMacroNodeNextPosition(
        const std::unique_ptr<ArkAST> &arkAst, const Ptr<NameReferenceExpr> &semaCacheExpr) const;

    /**
     * complete macro-modified field in macro node, ex:
     * @State var a: AA = AA()
     * a.[item]
     *
     * @param input ArkTs
     * @param pos Position
     * @param prefix std::string
     * @param env CompletionEnv
     * @param expr Expr
     */
    void NestedMacroComplete(const ArkAST &input, const Position &pos, const std::string &prefix,
                              CompletionEnv &env, Ptr<Expr> expr);

    void GetTyFromMacroCallNodes(Ptr<Expr> expr, std::unique_ptr<ArkAST> arkAst,
        Ptr<Ty> &ty, Ptr<NameReferenceExpr> &resExpr);

    void CompleteByReferenceTarget(const Position &pos, const std::string &prefix, CompletionEnv &env,
        const Ptr<Expr> &expr, const Ptr<NameReferenceExpr> &resExpr);

    Cangjie::ASTContext *context = nullptr;

    CompletionResult &result;

    Cangjie::ImportManager *importManager = nullptr;

    std::string packageNameForPath;

    std::string curFilePath;

    std::set<std::string> usedPkg = {};

    bool isEnumCtor = false;

    SyscapCheck syscap;
};

using FindFunc = void (ark::DotCompleterByParse::*)(
    Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

class DotMatcher {
public:
    static DotMatcher &GetInstance()
    {
        static DotMatcher instance{};
        return instance;
    }

    void RegFunc(ASTKind astKind, const FindFunc &func) { container[astKind] = func; }

    FindFunc GetFunc(ASTKind astKind)
    {
        auto item = container.find(astKind);
        if (item == container.end()) {
            return nullptr;
        }
        return item->second;
    }

private:
    DotMatcher() = default;

    ~DotMatcher() = default;

    std::map<ASTKind, FindFunc> container{};
};
} // namespace ark

#endif // LSPSERVER_DOTCOMPLETERBYPARSE_H
