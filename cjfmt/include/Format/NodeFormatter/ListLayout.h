// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_LISTLAYOUT_H
#define CJFMT_LISTLAYOUT_H

namespace Cangjie::Format {
// Explicit expansion is identified by a closing delimiter on a separate line.
// Ordinary width-generated continuations keep the delimiter with the last item
// and therefore cannot become forced breaks on the next invocation.
template <typename Members> bool PreserveExpandedList(int closingLine, const Members& members)
{
    constexpr size_t minimumExpandedMembers = 2;
    return members.size() >= minimumExpandedMembers && closingLine != members.back()->end.line;
}
}
#endif
