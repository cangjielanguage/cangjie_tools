// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/LambdaBodyProcessor.h"

using namespace Cangjie::Format;

void LambdaBodyProcessor::DocToString(std::string&, int& pos, std::pair<Doc, Mode>& current,
    std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    int width = MeasureFlat(current.first);
    int suffix = MeasureInlineSuffix(leftCmd);
    bool expand = current.first.members.size() == 1 &&
        (width < 0 || suffix < 0 || pos + width + suffix > options.lineLength);
    if (expand) {
        leftCmd.emplace_back(Doc(DocType::LINE, current.first.indent, ""), Mode::MODE_FLAT);
    }
    for (auto it = current.first.members.rbegin(); it != current.first.members.rend(); ++it) {
        leftCmd.emplace_back(*it, Mode::MODE_FLAT);
    }
    if (expand) {
        leftCmd.emplace_back(Doc(DocType::LINE, current.first.indent + 1, ""), Mode::MODE_FLAT);
    }
}
