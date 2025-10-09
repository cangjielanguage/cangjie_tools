// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_RENAMEIMPL_H
#define LSPSERVER_RENAMEIMPL_H

#include <algorithm>
#include <vector>
#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../ArkASTWorker.h"
#include "../../common/Utils.h"
#include "../../logger/Logger.h"
#include "cangjie/AST/Match.h"
#include "cangjie/AST/Searcher.h"

namespace ark {
using EditMap = std::unordered_map<std::string, std::set<TextEdit>>;
struct DocumentChanges {
    EditMap defineEditMap;
    EditMap usersEditMap;
};

class RenameImpl {
public:
    static std::string Rename(const ArkAST &ast, std::vector<TextDocumentEdit> &result, Cangjie::Position pos,
                              const std::string &newName, Callbacks &callback);

    static void RenameByIndex(lsp::SymbolID id, DocumentChanges &documentChanges, const std::string &newName);

    static std::string GetRealName(std::vector<TextDocumentEdit> &result, Callbacks &cb,
                                   ark::DocumentChanges &documentChanges);

    static void GetLocalVarUesage(Ptr<Decl> decl, const ArkAST &ast, ark::DocumentChanges &documentChanges,
                                  const std::string &newName);

    static void UpdateDefineMap(ark::DocumentChanges &documentChanges, std::string file, TextEdit t);

    static void UpdateUserMap(ark::DocumentChanges &documentChanges, std::string file, TextEdit t);

    static void HandleGeneric(Ptr<Decl> defineDecl, const ArkAST &ast, DocumentChanges &documentChanges,
                              const std::string &newName, const std::vector<Symbol *> &syms);

    static std::string curFilePath;
};
} // namespace ark

#endif // LSPSERVER_RENAMEIMPL_H
