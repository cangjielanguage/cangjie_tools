// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/ArgsProcessor.h"
#include "Format/ASTToFormatSource.h"
#include <string>
#include <utility>
#include <vector>
using namespace Cangjie::Format;

namespace {
void IncreaseIndent(Doc& doc)
{
    for (auto& member : doc.members) {
        member.indent++;
        IncreaseIndent(member);
    }
}
} // namespace

void ArgsProcessor::SoftLineProcessor(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, size_t& i, bool& haveNotChangedLine)
{
    std::pair<Doc, Mode> next(current.first.members[i + 1], current.second);
    if (!Fits(next, options.lineLength - pos)) {
        formatted += options.newLine;
        formatted += std::string(current.first.members[i].indent * options.indentWidth, ' ');
        pos = current.first.members[i].indent * options.indentWidth;
        for (size_t j = i + 1; j < current.first.members.size(); ++j) {
            if (current.first.members[j].type == DocType::FUNC_ARG && haveNotChangedLine) {
                current.first.members[j].indent++;
                IncreaseIndent(current.first.members[j]);
            }
        }
        haveNotChangedLine = false;
    }
}

void ArgsProcessor::SoftLineWithSpaceProcessor(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, size_t& i, bool& haveNotChangedLine)
{
    std::pair<Doc, Mode> next(current.first.members[i + 1], current.second);
    if (!Fits(next, options.lineLength - pos)) {
        formatted += options.newLine;
        formatted += std::string(current.first.members[i].indent * options.indentWidth, ' ');
        pos = current.first.members[i].indent * options.indentWidth;
        for (size_t j = i + 1; j < current.first.members.size(); ++j) {
            if (current.first.members[j].type == DocType::FUNC_ARG && haveNotChangedLine) {
                current.first.members[j].indent++;
                IncreaseIndent(current.first.members[j]);
            }
        }
        haveNotChangedLine = false;
    } else {
        if (next.first.type != DocType::LINE) {
            formatted += " ";
            pos += 1;
        }
    }
}

void ArgsProcessor::DocToString(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    bool haveNotChangedLine = true;
    for (size_t i = 0; i < current.first.members.size(); ++i) {
        if (current.first.members[i].type == DocType::SOFTLINE) {
            if (current.first.members.size() == 1) {
                return;
            }
            SoftLineProcessor(formatted, pos, current, i, haveNotChangedLine);
            continue;
        }
        if (current.first.members[i].type == DocType::SOFTLINE_WITH_SPACE) {
            SoftLineWithSpaceProcessor(formatted, pos, current, i, haveNotChangedLine);
            continue;
        }
        astToFormatSource.DocToString(current.first.members[i], pos, formatted);
    }
}
