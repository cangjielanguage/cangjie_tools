// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_DOC_H
#define CJFMT_DOC_H

#include <string>
#include <vector>

namespace Cangjie::Format {

struct FormattingOptions {
    int indentWidth{4};        // default indentation width
    int lineLength{120};       // default line width
    std::string newLine{"\n"}; // default newline character
    bool allowMultiLineMethodChain = false;
    int multipleLineMethodChainLevel = 5;
    bool multipleLineMethodChainOverLineLength = true;
};

struct FuncOptions {
    bool patternOrEnum;
    bool isLambda;
    bool isSpawn;
    bool isMultipleLineMacroExpendParam;
    bool isMethodChainning;
    bool isInsideBuildNode;

    explicit FuncOptions(bool patternOrEnum = false, bool isLambda = false, bool isSpawn = false,
        bool isMultipleLineMacroExpendParam = false, bool isMethodChainning = false, bool isInsideBuildNode = false)
        : patternOrEnum(patternOrEnum),
          isLambda(isLambda),
          isSpawn(isSpawn),
          isMultipleLineMacroExpendParam(isMultipleLineMacroExpendParam),
          isMethodChainning(isMethodChainning),
          isInsideBuildNode(isInsideBuildNode)
    {
    }
};

enum class DocType {
    FILE,
    CONCAT,
    GROUP,
    ARGS,
    FUNC_ARG,
    LINE_DOT,
    DOT,
    LAMBDA,
    LAMBDA_BODY,
    MEMBER_ACCESS,
    CHOICE,
    FILL,
    IF_BREAK,
    BREAK_PARENT,
    JOIN,
    LINE,
    SOFTLINE_WITH_SPACE,
    SOFTLINE,
    HARDLINE,
    LITERALLINE,
    LINE_SUFFIX,
    LINE_SUFFIX_BOUNDARY,
    INDENT,
    ALIGN,
    STRING,
    DOC_COMMENT,
    LINE_COMMENT,
    SUFFIX_COMMENT,
    SEPARATE,
    INVALID,
};

enum class Mode {
    MODE_FLAT,  // don't do line break, use space
    MODE_GROUP_FLAT, // a semantic Group has selected its complete flat layout
    MODE_BREAK, // do line break as much as possible
    INVALID,
};

// Legacy groups are transparent. Semantic groups describe complete syntax nodes and protect their
// flat layout; Fill groups permit independent breaks between complete operands or list items.
// LAMBDA_HEADER groups decide parameter breaks from the parameters' flat width at the current column,
// independently of the following body or method chain.
enum class LayoutPolicy { LEGACY, GROUP, FILL, LAMBDA_HEADER };
enum class BreakKind { GENERIC, LOGICAL, BINARY, LIST, OPEN, MEMBER, ASSIGNMENT };

struct Doc {
    DocType type = DocType::INVALID;
    int indent{};
    std::string value;
    std::vector<Doc> members;
    LayoutPolicy layout = LayoutPolicy::LEGACY;
    bool rebaseToOutputLine = true;
    bool isStringLiteral = false;
    BreakKind breakKind = BreakKind::GENERIC;
    int breakPenalty = 0; // zero selects the default for breakKind
    size_t choiceBreakAt = 0; // index of RHS for the assignment fallback candidate
    Doc() = default;
    Doc(DocType ty, int ind, std::string val)
    {
        type = ty;
        indent = ind;
        value = std::move(val);
    }
    Doc(DocType ty, int ind, std::vector<Doc> mem)
    {
        type = ty;
        indent = ind;
        members = std::move(mem);
    }
};
} // namespace Cangjie::Format

#endif // CJFMT_DOC_H
