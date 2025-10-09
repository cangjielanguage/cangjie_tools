// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_SEMANTICTOKENS_H
#define LSPSERVER_SEMANTICTOKENS_H

#include "../../../json-rpc/Protocol.h"
#include "SemanticHighlightImpl.h"
#include "../../ArkAST.h"

// An adapter for the original semantic highlight class SemanticHighlightImpl and do some conversion
namespace ark {
class SemanticTokensAdaptor {
public:
    const static std::vector<std::string> TOKEN_TYPES;
    const static std::vector<std::string> TOKEN_MODIFIERS;
    // Outer interface
    static void FindSemanticTokens(const ArkAST &ast, SemanticTokens &result, unsigned int fileID);
private:
    // A helper struct to sort raw data
    struct SemanticTokensFormat {
        int line;
        int startChar;
        int length;
        int tokenType;
        int tokenTypeModifier;
        bool operator<(const SemanticTokensFormat tokens) const
        {
            // There can't be two different tokens standing the same startPos and line number
            if (line == tokens.line) {
                return startChar < tokens.startChar;
            }
            return line < tokens.line;
        }
        bool operator==(const SemanticTokensFormat tokens) const
        {
            return line == tokens.line && startChar == tokens.startChar && length == tokens.length &&
                   tokenType == tokens.tokenType && tokenTypeModifier == tokens.tokenTypeModifier;
        }
    };

    static void FromHighlightToSemaTokens(const std::vector<SemanticHighlightToken> &originVec,
        SemanticTokens &newVec);

    static std::vector<int> TokenKindConversion(const HighlightKind &highlightKind);

    static void ReadyForSemanticTokenMsg(const std::set<SemanticTokensFormat> &semaTokensRawSet,
        SemanticTokens &semaTokens);
    static const std::map<HighlightKind, std::vector<int>> HIGHLIGHT_TO_TOKEN_KIND_MAP;
};
}
#endif // LSPSERVER_SEMANTICTOKENS_H
