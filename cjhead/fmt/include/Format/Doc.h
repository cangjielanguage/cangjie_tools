// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_DOC_H
#define CJFMT_DOC_H

#include <string>
#include <vector>
#include <memory>
namespace Cangjie::Format {

struct FormattingOptions {
    int indentWidth{4};        // 默认缩进宽度
    int lineLength{120};       // 默认行宽
    std::string newLine{"\n"}; // 默认换行符
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
    MODE_BREAK, // do line break as much as possible
    INVALID,
};

struct Doc {
    DocType type = DocType::INVALID;
    int indent{};
    std::string value;
    std::vector<Doc> members;
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
