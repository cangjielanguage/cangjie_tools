// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/ArgsProcessor.h"
#include "Format/ASTToFormatSource.h"

using namespace Cangjie::Format;

namespace {
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

void ArgsProcessor::DocToString(
    std::string&, int&, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
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
    // Use the shared pure planner. Keep the surrounding command stack visible
    // rather than rendering each argument in a separate, suffix-blind stack.
    current.first.type = DocType::FILL;
    current.first.layout = LayoutPolicy::FILL;
    leftCmd.emplace_back(std::move(current));
}
