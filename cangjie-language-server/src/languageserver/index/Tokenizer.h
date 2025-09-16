// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_INDEX_TOKENIZER_H
#define LSPSERVER_INDEX_TOKENIZER_H

#include <functional>
#include <string>

namespace ark {
namespace lsp {

using TokenizerCallback =
    std::function<void(std::string_view /* Token */, size_t /* Offset */)>;

using TokenizerFunction =
    std::function<void(std::string_view /* Text */, TokenizerCallback)>;

TokenizerFunction GetIdentifierTokenizer();


} // namespace lsp
} // namespace ark

#endif // LSPSERVER_INDEX_TOKENIZER_H
