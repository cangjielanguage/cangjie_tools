// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_COMMENTHANDLER_H
#define CJFMT_COMMENTHANDLER_H

#include "cangjie/Lex/Token.h"
#include "cangjie/Basic/SourceManager.h"

#include <string>
#include <vector>

namespace Cangjie::Format {

std::string InsertComments(const std::vector<Cangjie::Token> &originalTokens,
    const std::vector<Cangjie::Token> &formattedTokens, Cangjie::SourceManager &sm);

} // namespace Cangjie::Format

#endif // CJFMT_COMMENTHANDLER_H
