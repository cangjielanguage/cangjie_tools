// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/ChoiceProcessor.h"

using namespace Cangjie::Format;

void ChoiceProcessor::DocToString(std::string&, int& pos, std::pair<Doc, Mode>& current,
    std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    Doc selected = current.first;
    selected.type = DocType::CONCAT;
    // The preferred layout owns its internal breaks. Use the higher-cost
    // assignment fallback only when even its first complete chunk overflows.
    std::vector<std::pair<Doc, Mode>> trial{{selected, Mode::MODE_FLAT}};
    int firstLine = MeasureInlineSuffix(trial);
    auto split = selected.choiceBreakAt;
    bool overlongString = false;
    if (split > 0 && split < selected.members.size()) {
        const auto& right = selected.members[split];
        overlongString = right.isStringLiteral && MeasureFlat(right) > options.lineLength;
    }
    if (!overlongString && firstLine >= 0 && pos + firstLine > options.lineLength &&
        split > 0 && split < selected.members.size()) {
        Doc* last = &selected.members[split - 1];
        while (!last->members.empty()) {
            last = &last->members.back();
        }
        if (last->type == DocType::STRING && !last->value.empty() && last->value.back() == ' ') {
            last->value.pop_back();
        }
        selected.members.insert(selected.members.begin() + split, Doc(DocType::LINE, selected.indent + 1, ""));
    }
    leftCmd.emplace_back(std::move(selected), Mode::MODE_FLAT);
}
