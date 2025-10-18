// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_POSITIONRESOLVER_H
#define LSPSERVER_POSITIONRESOLVER_H

#include "cangjie/AST/Node.h"
#include "../../json-rpc/Common.h"
#include "../../json-rpc/Protocol.h"
#include "../ArkAST.h"
#include "cangjie/Basic/Display.h"

namespace ark {
enum class UTF8Kind {
    BYTE_ONE = 0,    // 0xxxxxxx
    BYTE_TWO = 1,    // 110xxxxx 10xxxxxx
    BYTE_THREE = 2,  // 1110xxxx 10xxxxxx 10xxxxxx
    BYTE_FOUR = 3    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
};

bool IsUTF8(const std::string &str);

std::basic_string<char32_t> UTF8ToChar32(const std::string &str);

std::string Char32ToUTF8(const std::basic_string<char32_t>& str);

int GetFirstTokenOnCurLine(const std::vector<Cangjie::Token> &tokens, int declLine);

int GetLastTokenOnCurLine(const std::vector<Cangjie::Token> &tokens, int declLine);

Cangjie::Position TransformFromChar2IDE(Cangjie::Position pos);

Cangjie::Position PosFromIDE2Char(Cangjie::Position pos);

Range TransformFromChar2IDE(Range range);

Range TransformFromIDE2Char(Range range);

bool PositionInCurToken(int line, int column, const Cangjie::Token &token);

int LineOfCommentEnd(const Cangjie::Token &token, std::string &lastString);

void InterpStringInMultiString(const std::vector<Cangjie::Token> &tokens, Cangjie::Position &pos,
                               const Cangjie::AST::Node &node, bool isIDEToUTF8);

bool IsMultiLine(const Cangjie::Token &token);

int RedundantCharacterOfMultiLineString(const std::vector<Cangjie::Token> &tokens, const Cangjie::Position &pos,
                                        int preBegin);

void PositionIDEToUTF8(const std::vector<Cangjie::Token> &tokens, Cangjie::Position &pos,
                       const Cangjie::AST::Node &node);

void PositionIDEToUTF8ForC(const ArkAST &input, Cangjie::Position &pos);

void PositionUTF8ToIDE(const std::vector<Cangjie::Token> &tokens, Cangjie::Position &pos,
                       const Cangjie::AST::Node &node);

int CountUnicodeCharacters(const std::string& utf8Str);

void UpdateRange(const std::vector<Cangjie::Token> &tokens, Range &range, const Cangjie::AST::Node &node,
                 bool needUpdateByName = true);
} // namespace ark

#endif // LSPSERVER_POSITIONRESOLVER_H
