// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_ARKAST_H
#define LSPSERVER_ARKAST_H

#include <cstdint>
#include <sstream>
#include "../json-rpc/Protocol.h"
#include "../json-rpc/URI.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Match.h"
#include "cangjie/AST/Symbol.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Basic/Match.h"
#include "cangjie/Lex/Token.h"
#include "cangjie/Modules/ImportManager.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/Utils/FileUtil.h"


#include "DocCache.h"

namespace ark {
using namespace Cangjie;
using namespace Cangjie::AST;
struct ParseInputs {
    std::string fileName;
    std::string contents;
    std::int64_t version = 0;
    bool forceRebuild = false;

    ParseInputs(std::string fileName, std::string ctent, std::int64_t v, bool forceRebuild = false)
        : fileName(std::move(fileName)), contents(std::move(ctent)), version(v),
          forceRebuild(forceRebuild)
    {
    }

    ParseInputs() {}
    ParseInputs &operator=(const ParseInputs &input)
    {
        if (this == &input) {
            return *this;
        }

        this->fileName = input.fileName;
        this->contents = input.contents;
        this->version = input.version;
        this->forceRebuild = input.forceRebuild;
        return *this;
    }

    ParseInputs(const ParseInputs &input)
    {
        this->fileName = input.fileName;
        this->contents = input.contents;
        this->version = input.version;
        this->forceRebuild = input.forceRebuild;
    }
};

struct PackageInstance {
    PackageInstance(Cangjie::DiagnosticEngine &d, Cangjie::ImportManager &imp)
        : package(nullptr), diag(d), importManager(imp), ctx(nullptr)
    {
    }

    ~PackageInstance() {}

    Ptr<const Cangjie::AST::Package> package;
    Cangjie::DiagnosticEngine &diag;
    Cangjie::ImportManager &importManager;
    Cangjie::ASTContext *ctx;
};

struct ArkAST {
    ArkAST(const std::pair<std::string, std::string> &paths,
           Ptr<const File> node,
           Cangjie::DiagnosticEngine &diagEngine,
           PackageInstance *pkgInstance,
           Cangjie::SourceManager *sm)
        : diag(diagEngine), file(node), packageInstance(pkgInstance), sourceManager(sm)
    {
        DoLexer(paths.second, paths.first);
    }

    ~ArkAST() {}

    int GetCurTokenByPos(const Cangjie::Position &pos,
                         int start,
                         int end,
                         bool isForRename = false) const;

    int GetCurToken(const Cangjie::Position &pos, int start, int end) const;

    int GetCurTokenByStartColumn(const Cangjie::Position &pos, int start, int end) const;

    int GetCurTokenSkipSpace(const Cangjie::Position &pos, int start, int end, int lastEnd) const;

    bool IsFilterToken(const Position &pos) const;

    bool IsFilterTokenInHighlight(const Position &pos) const;

    void DoLexer(const std::string &contents, const std::string &fileName);

    bool CheckTokenKind(Cangjie::TokenKind tokenKind, bool isForRename) const;

    bool CheckTokenKindWhenRenamed(Cangjie::TokenKind tokenKind) const;

    Ptr<Cangjie::AST::Decl> GetDeclByPosition(const Cangjie::Position &originPos,
                                              std::vector<Cangjie::AST::Symbol *> &syms,
                                              std::vector<Ptr<Decl>> &decls,
                                              const std::pair<bool, bool> &isMacroOrRename = {
                                                  false, false}) const;

    Ptr<Cangjie::AST::Decl> GetDeclByPosition(const Cangjie::Position &originPos) const;

    std::vector<Ptr<Decl>> GetOverloadDecls(const ark::Token token) const;

    Ptr<Cangjie::AST::Decl> FindDeclByNode(Ptr<Cangjie::AST::Node> node) const;

    Ptr<Cangjie::AST::Node> GetNodeBySymbols(const ark::ArkAST &,
                                             Ptr<Cangjie::AST::Node> node,
                                             const std::vector<Cangjie::AST::Symbol *> &,
                                             const std::string &query,
                                             const size_t) const;

    std::vector<Ptr<Decl>> FindRealDecl(const ark::ArkAST &nowAst,
                                        const std::vector<Cangjie::AST::Symbol *> &syms,
                                        const std::string &query = "",
                                        const Cangjie::Position &macroPos = {0, 0, 0},
                                        const std::pair<bool, bool> &isMacroOrRename = {
                                            false, false}) const;

    Ptr<Cangjie::AST::Decl> GetDelFromType(Ptr<const Cangjie::AST::Type> type) const;

    Ptr<Cangjie::AST::Decl> FindRealGenericParamDeclForExtend(
        const std::string &genericParamName, const std::vector<Cangjie::AST::Symbol *> syms) const;

    bool CheckInQuote(Ptr<const Node> node, const Cangjie::Position &pos) const;

    Cangjie::DiagnosticEngine &diag;
    std::vector<Cangjie::Token> tokens;
    Ptr<const Cangjie::AST::File> file = nullptr;
    PackageInstance *packageInstance;
    SourceManager *sourceManager{nullptr};
    ArkAST *semaCache = nullptr;
    unsigned int fileID = 0;
};
} // namespace ark

#endif // LSPSERVER_ARKAST_H
