// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "SQLiteAPI.h"

#include <string_view>

namespace sqldb {
namespace impl {

template <typename Function>
struct Tokenizer {
    Function Tokenize;
};

template <typename Tokenizer>
auto &tokenize(Fts5Tokenizer *Tok)
{
    return reinterpret_cast<Tokenizer *>(Tok)->Tokenize;
}

template <typename Tokenizer>
int tokenize(Fts5Tokenizer *Tok,
    void *Ctx,
    int,
    const char *Text,
    int TextLen,
    int (*Callback)(void *, int, const char *, int, int, int))
{
    tokenize<Tokenizer>(Tok)(std::string_view(Text, TextLen), [&](std::string_view Token, size_t Pos) {
        Callback(Ctx, 0, Token.data(), Token.size(), Pos, Pos + Token.size());
    });
    return sqlite::OK;
}

} // namespace impl
} // namespace sqldb