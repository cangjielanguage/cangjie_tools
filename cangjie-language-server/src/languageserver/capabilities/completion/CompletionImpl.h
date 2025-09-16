// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_COMPLETION_H
#define LSPSERVER_COMPLETION_H

#include <cstdint>
#include <iostream>
#include <vector>
#include "../../../json-rpc/Protocol.h"
#include "../../../json-rpc/CompletionType.h"
#include "../../common/Utils.h"
#include "../../logger/Logger.h"
#include "cangjie/Lex/Token.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/Basic/Match.h"
#include "cangjie/AST/Match.h"
#include "cangjie/AST/Symbol.h"
#include "cangjie/Utils/FileUtil.h"
#include "cangjie/Modules/ImportManager.h"
#include "../../ArkAST.h"

namespace ark {

enum class SortType : uint8_t {
    NORMAL_SYM,
    AUTO_IMPORT_SYM,
    KEYWORD,
};

struct CodeCompletion {
    bool show = true;
    bool isEnumCtor = false; // Special for enum constructor in different match expr
    bool deprecated = false;

    SortType sortType = SortType::NORMAL_SYM; // for completion list sort
    CompletionItemKind kind = CompletionItemKind::CIK_MISSING;
    std::string name;
    std::string label;
    std::string detail;
    std::string insertText;
    std::string container;
    uint8_t itemDepth = 0;
    std::optional<std::vector<TextEdit>> additionalTextEdits;
    ark::lsp::SymbolID id = 0;

    [[nodiscard]] CompletionItem Render(const std::string &sortText, const std::string &prefix) const;
};

struct CompletionResult {
    std::vector<CodeCompletion> completions {};
    uint8_t cursorDepth = 0;
    std::unordered_set<ark::lsp::SymbolID> normalCompleteSymID {};
    std::unordered_set<ark::lsp::SymbolID> importDeclsSymID {};
};

class CompletionImpl {
public:
    static void CodeComplete(const ArkAST &input, Cangjie::Position pos,
                             CompletionResult &result, std::string &prefix);

    static std::string GetChainedNameComplex(const ArkAST &input, int start, int end);

    static bool IsPreambleComplete(const ArkAST &input, const TokenKind firstTokenKind,
                                   const int firstTokenIndexOnLine);

    static bool IsPreamble(const ArkAST &input, Cangjie::Position pos);

    static Token curToken;

    static bool needImport;

    static std::unordered_set<std::string> externalImportSym;

private:
    static void FasterComplete(const ArkAST &input, Cangjie::Position pos,
                              CompletionResult &result, int index, std::string &prefix);

    static void NormalParseImpl(const ArkAST &input, const Cangjie::Position &pos,
                                CompletionResult &result, int index, std::string &prefix);

    static void AutoImportPackageComplete(const ArkAST &input, CompletionResult &result, const std::string &prefix);

    static void HandleExternalSymAutoImport(CompletionResult &result, const std::string &pkg, const lsp::Symbol &sym,
        const lsp::CompletionItem &completionItem, Range textEditRange);
};
} // namespace ark

#endif // LSPSERVER_COMPLETION_H
