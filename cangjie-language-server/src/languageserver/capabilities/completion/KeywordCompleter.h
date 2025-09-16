// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_KEYWORDCOMPLETER_H
#define LSPSERVER_KEYWORDCOMPLETER_H

#include "CompletionImpl.h"
#include "cangjie/Lex/AnnotationToken.h"

namespace ark {

struct CodeSnippet {
    std::string keyWord;
    std::string label;
    std::string snippet;
};

class KeywordCompleter {
public:
    static void Complete(CompletionResult &result);
    static void AddKeyWord(
        const char *tokens[], int size, ark::CompletionResult &result, std::function<bool(const char *)> condition);
    static void AddKeyWordByLSP(ark::CompletionResult& result);
    static std::unordered_set<TokenKind> keyWordKinds;
    static std::unordered_set<TokenKind> declKeyWordKinds;
private:
    static std::vector<CodeSnippet> codeSnippetList;
    static std::vector<std::string> keyWordFromLSP;
};
} // namespace ark

#endif // LSPSERVER_KEYWORDCOMPLETER_H
