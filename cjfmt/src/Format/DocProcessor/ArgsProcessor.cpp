// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/ArgsProcessor.h"
#include "Format/ASTToFormatSource.h"

using namespace Cangjie::Format;

namespace {
void IncreaseIndent(Doc& doc)
{
    for (auto& member : doc.members) {
        member.indent++;
        IncreaseIndent(member);
    }
}

void AppendInlineSuffix(Doc& chunk, std::vector<std::pair<Doc, Mode>>& commands)
{
    while (!commands.empty() && commands.back().second == Mode::MODE_FLAT) {
        auto type = commands.back().first.type;
        if (type == DocType::CONCAT || type == DocType::GROUP || type == DocType::FILL ||
            type == DocType::FUNC_ARG) {
            // A postfix expression (for example an array index) can contain nested Docs. Expose its
            // inline prefix, preserving the next soft/hard break as the boundary of this suffix.
            auto command = std::move(commands.back());
            commands.pop_back();
            for (auto it = command.first.members.rbegin(); it != command.first.members.rend(); ++it) {
                commands.emplace_back(std::move(*it), command.second);
            }
        } else if (type == DocType::STRING || type == DocType::DOT || type == DocType::BREAK_PARENT ||
            type == DocType::ARGS || type == DocType::MEMBER_ACCESS) {
            chunk.members.emplace_back(std::move(commands.back().first));
            commands.pop_back();
        } else {
            break;
        }
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
    bool addSpace = next.first.type != DocType::LINE && next.first.type != DocType::HARDLINE;
    auto rem = options.lineLength - pos - (addSpace ? 1 : 0);
    if (!Fits(next, rem)) {
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
        if (addSpace) {
            formatted += " ";
            pos += 1;
        }
    }
}

void ArgsProcessor::DocToString(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    // Keep commas and closing delimiters in the same candidate as their argument. Nested expression
    // printers otherwise run with an isolated command stack and cannot reserve the columns for them.
    std::vector<Doc> members;
    for (auto& member : current.first.members) {
        if (member.type == DocType::SOFTLINE || member.type == DocType::SOFTLINE_WITH_SPACE) {
            members.emplace_back(std::move(member));
        } else if (members.empty() || members.back().type == DocType::SOFTLINE ||
            members.back().type == DocType::SOFTLINE_WITH_SPACE) {
            Doc chunk(member.type == DocType::FUNC_ARG ? DocType::FUNC_ARG : DocType::CONCAT, member.indent, "");
            chunk.members.emplace_back(std::move(member));
            members.emplace_back(std::move(chunk));
        } else {
            members.back().members.emplace_back(std::move(member));
        }
    }
    if (!members.empty() && members.back().type != DocType::SOFTLINE &&
        members.back().type != DocType::SOFTLINE_WITH_SPACE) {
        AppendInlineSuffix(members.back(), leftCmd);
    }
    current.first.members = std::move(members);
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
