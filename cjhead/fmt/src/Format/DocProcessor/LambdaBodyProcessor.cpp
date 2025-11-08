// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/LambdaBodyProcessor.h"
#include "Format/ASTToFormatSource.h"
#include <string>
#include <utility>
#include <vector>

using namespace Cangjie::Format;

namespace {
bool hasLineType(Doc& doc)
{
    for (auto& member : doc.members) {
        if (member.type == DocType::LINE) {
            return true;
        }
        if (hasLineType(member)) {
            return true;
        }
    }
    return false;
}

void IncreaseIndent(Doc& doc)
{
    for (auto& member : doc.members) {
        member.indent++;
        IncreaseIndent(member);
    }
}
} // namespace

void LambdaBodyProcessor::DocToString(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    if (current.first.members.size() == 1) {
        auto hasLine = hasLineType(current.first);
        auto rem = options.lineLength - pos;
        bool overLength;
        if (!hasLine) {
            auto length = CalculateDocLen(current.first);
            overLength = rem - length <= 0 ? true : false;
        }
        if (hasLine || overLength) {
            formatted += options.newLine;
            formatted += std::string((current.first.indent + 1) * options.indentWidth, ' ');
            pos = (current.first.indent + 1) * options.indentWidth;
        }
        for (auto& member : current.first.members) {
            astToFormatSource.DocToString(member, pos, formatted);
        }
        if (hasLine || overLength) {
            formatted += options.newLine;
            formatted += std::string(current.first.indent * options.indentWidth, ' ');
            pos = current.first.indent * options.indentWidth;
        }
    } else {
        for (auto& member : current.first.members) {
            astToFormatSource.DocToString(member, pos, formatted);
        }
    }
}