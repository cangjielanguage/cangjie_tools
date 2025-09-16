// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_COMPLETIONTYPE_H
#define LSPSERVER_COMPLETIONTYPE_H

#include <optional>
#include "Common.h"

/**
 * According to the language service protocol to create structure
 * see https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#baseProtocol
 */
namespace ark {

enum class CompletionTriggerKind {
    INVOKED = 1,

    TRIGGER_CHAR = 2,
};

struct CompletionContext {
    CompletionTriggerKind triggerKind = CompletionTriggerKind::INVOKED;

    std::string triggerCharacter = "";

    CompletionContext(): triggerKind(CompletionTriggerKind::INVOKED), triggerCharacter("") {};
};

enum class CompletionItemKind {
    CIK_MISSING = 0,
    CIK_METHOD = 2,
    CIK_FUNCTION = 3,
    CIK_CONSTRUCTOR = 4,
    CIK_VARIABLE = 6,
    CIK_CLASS = 7,
    CIK_INTERFACE = 8,
    CIK_MODULE = 9,
    CIK_ENUM = 13,
    CIK_KEYWORD = 14,
    CIK_STRUCT = 22,
};

enum class InsertTextFormat {
    MISSING = 0,

    PLAIN_TEXT = 1,

    SNIPPET = 2,
};

struct TextEdit {
    Range range;

    std::string newText;

    TextEdit() {};

    TextEdit(const Range &range, const std::string &newText) : range(range), newText(newText)
    {
    }

    bool operator<(const TextEdit &rhs) const
    {
        return std::tie(this->range, this->newText) < std::tie(rhs.range, rhs.newText);
    }
};

struct CompletionItem {
    std::string label = "";

    CompletionItemKind kind = CompletionItemKind::CIK_MISSING;

    std::string detail = "";

    std::string documentation = "";

    std::string sortText = "";

    std::string filterText = "";

    std::string insertText = "";

    InsertTextFormat insertTextFormat = InsertTextFormat::MISSING;

    std::optional<TextEdit> textEdit;

    std::optional<std::vector<TextEdit>> additionalTextEdits;

    bool deprecated = false;

    CompletionItem()
        : label(""),
          kind(CompletionItemKind::CIK_MISSING),
          detail(""),
          documentation(""),
          sortText(""),
          filterText(""),
          insertText(""),
          insertTextFormat(InsertTextFormat::MISSING),
          textEdit(),
          deprecated(false)
    {};
};
}
#endif // LSPSERVER_COMPLETIONTYPE_H
