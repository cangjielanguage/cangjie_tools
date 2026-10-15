// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_COMMENTLAYOUT_H
#define CJFMT_COMMENTLAYOUT_H
#include "Format/Doc.h"
#include "cangjie/Lex/Token.h"

namespace Cangjie::Format {
// Width-only annotations; CommentHandler retains ownership and emits the text.
void AddCommentBudgets(Doc& doc, const std::vector<Token>& tokens);
}
#endif
