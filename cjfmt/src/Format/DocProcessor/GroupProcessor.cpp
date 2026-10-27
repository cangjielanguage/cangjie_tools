// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/GroupProcessor.h"
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::GroupProcessor::DocToString(std::string &, int &pos, std::pair<Doc, Mode> &current,
    std::vector<std::pair<Doc, Mode>> &leftCmd)
{
    auto mode = current.second;
    if (current.first.layout == LayoutPolicy::GROUP) {
        int width = MeasureFlat(current.first);
        int suffix = MeasureInlineSuffix(leftCmd);
        mode = width >= 0 && suffix >= 0 && pos + width + suffix <= options.lineLength
            ? Mode::MODE_GROUP_FLAT : Mode::MODE_BREAK;
    } else if (current.first.layout == LayoutPolicy::LAMBDA_HEADER) {
        // Lambda parameters form a semantic header. Their comma breaks are
        // decided from the header itself; the body and its method chain have
        // their own groups and must not force an early parameter break.
        int width = MeasureFlat(current.first);
        mode = width >= 0 && pos + width <= options.lineLength
            ? Mode::MODE_GROUP_FLAT : Mode::MODE_BREAK;
    }
    for (auto it = current.first.members.rbegin(); it != current.first.members.rend(); ++it) {
        leftCmd.emplace_back(*it, mode);
    }
}
}
