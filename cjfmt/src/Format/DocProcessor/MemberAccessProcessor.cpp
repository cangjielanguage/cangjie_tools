// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/MemberAccessProcessor.h"
#include "Format/ASTToFormatSource.h"

#include <algorithm>

using namespace Cangjie::Format;

namespace {
bool ContainsForcedBreak(const Doc& doc)
{
    switch (doc.type) {
        case DocType::LINE:
        case DocType::HARDLINE:
        case DocType::LINE_DOT:
        case DocType::SEPARATE:
        case DocType::BREAK_PARENT:
        case DocType::LINE_SUFFIX_BOUNDARY:
        case DocType::LITERALLINE:
            return true;
        case DocType::STRING:
            // A raw multiline literal cannot be flattened. It is represented
            // as a STRING rather than a LINE doc, so detect its embedded line
            // ending here as well.
            return doc.value.find_first_of("\r\n") != std::string::npos;
        default:
            break;
    }
    return std::any_of(doc.members.begin(), doc.members.end(), [](const Doc& member) {
        return ContainsForcedBreak(member);
    });
}

void FlattenChain(const Doc& doc, std::vector<Doc>& parts)
{
    if (doc.type == DocType::MEMBER_ACCESS || doc.type == DocType::CONCAT ||
        (doc.type == DocType::GROUP && doc.layout == LayoutPolicy::LEGACY)) {
        for (const auto& member : doc.members) {
            FlattenChain(member, parts);
        }
    } else {
        parts.emplace_back(doc);
    }
}

bool g_expandTransparentCommand(std::vector<std::pair<Doc, Mode>>& commands)
{
    auto type = commands.back().first.type;
    if (type != DocType::CONCAT &&
        !(type == DocType::GROUP && commands.back().first.layout == LayoutPolicy::LEGACY)) {
        return false;
    }
    auto command = std::move(commands.back());
    commands.pop_back();
    for (auto it = command.first.members.rbegin(); it != command.first.members.rend(); ++it) {
        commands.emplace_back(std::move(*it), command.second);
    }
    return true;
}

void AppendParenthesizedCall(std::vector<Doc>& parts, std::vector<std::pair<Doc, Mode>>& commands)
{
    int depth = 0;
    do {
        // Expose comment-wrapped delimiters before counting parentheses, so
        // an annotated ')' cannot consume following statements or block ends.
        if (g_expandTransparentCommand(commands)) {
            continue;
        }
        auto part = std::move(commands.back().first);
        commands.pop_back();
        depth += part.value == "(" ? 1 : part.value == ")" ? -1 : 0;
        parts.emplace_back(std::move(part));
    } while (!commands.empty() && depth > 0);
}

void AppendCallSuffix(std::vector<Doc>& parts, std::vector<std::pair<Doc, Mode>>& commands)
{
    constexpr size_t trailingClosureParts = 2;
    if (!commands.empty() && commands.back().first.value == "(") {
        AppendParenthesizedCall(parts, commands);
    }
    // Keep trailing closures inside the method segment's ALIGN scope, anchored
    // before arguments wrap, so the closure inherits the method's base indent.
    if (commands.size() >= trailingClosureParts && commands.back().first.type == DocType::STRING &&
        commands.back().first.value == " " &&
        commands[commands.size() - trailingClosureParts].first.type == DocType::LAMBDA) {
        parts.emplace_back(std::move(commands.back().first));
        commands.pop_back();
        parts.emplace_back(std::move(commands.back().first));
        commands.pop_back();
    }
}

Doc BuildMemberChain(const std::vector<Doc>& parts, int indent, bool expand)
{
    Doc chain(DocType::FILL, indent, "");
    chain.layout = LayoutPolicy::FILL;
    Doc segment(DocType::ALIGN, indent, "");
    bool continuation = false;
    for (const auto& part : parts) {
        if (part.type == DocType::DOT || part.type == DocType::LINE_DOT) {
            if (continuation) {
                chain.members.emplace_back(std::move(segment));
                segment = Doc(DocType::ALIGN, indent, "");
            }
            continuation = true;
            bool forced = expand || part.type == DocType::LINE_DOT;
            Doc separator(forced ? DocType::LINE : DocType::SOFTLINE, indent + 1, "");
            separator.breakKind = BreakKind::MEMBER;
            chain.members.emplace_back(std::move(separator));
            chain.members.emplace_back(DocType::STRING, indent + 1, ".");
        } else {
            if (continuation) {
                segment.members.emplace_back(part);
            } else {
                chain.members.emplace_back(part);
            }
        }
    }
    if (continuation) {
        chain.members.emplace_back(std::move(segment));
    }
    return chain;
}
} // namespace

void MemberAccessProcessor::DocToString(std::string& formatted, int& pos, std::pair<Doc, Mode>& current,
    std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    std::vector<Doc> parts;
    FlattenChain(current.first, parts);
    AppendCallSuffix(parts, leftCmd);
    size_t count = 0;
    for (const auto& part : parts) {
        count += part.type == DocType::DOT || part.type == DocType::LINE_DOT;
    }
    auto firstDot = std::find_if(parts.begin(), parts.end(), [](const Doc& part) {
        return part.type == DocType::DOT || part.type == DocType::LINE_DOT;
    });
    std::vector<Doc> receiverParts(parts.begin(), firstDot);
    std::vector<Doc> chainParts(firstDot, parts.end());
    Doc receiver(DocType::CONCAT, current.first.indent, std::move(receiverParts));
    Doc suffixChain(DocType::CONCAT, current.first.indent, std::move(chainParts));
    astToFormatSource.DocToString(receiver, pos, formatted);
    int width = MeasureFlat(suffixChain);
    int suffix = MeasureInlineSuffix(leftCmd);
    bool chainHasForcedBreak = ContainsForcedBreak(suffixChain);
    // Print the complete receiver first: calls and closures can wrap while
    // printing. Use its actual final column, independent of source newlines,
    // to decide whether the following chain needs one method per line.
    // An outer expression may already force a break after this chain. In that
    // case its suffix has no usable flat width, but the chain still needs to
    // be checked against the current line by itself.
    int suffixWidth = suffix < 0 ? 0 : suffix;
    bool expand = count > 1 && options.multipleLineMethodChainOverLineLength &&
        (chainHasForcedBreak || (width >= 0 && pos + width + suffixWidth > options.lineLength));
    auto chain = BuildMemberChain(suffixChain.members, current.first.indent, expand);
    leftCmd.emplace_back(std::move(chain), Mode::MODE_FLAT);
}
