// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/DocProcessor.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

bool DocProcessor::Fits(const std::pair<Doc, Mode>& next, int rem)
{
    return Fits(std::vector<std::pair<Doc, Mode>>{next}, rem);
}

bool DocProcessor::Fits(const std::vector<std::pair<Doc, Mode>>& commands, int rem)
{
    // The first pending Doc is a complete flat candidate. Include its following suffix up to the next
    // outer break opportunity, so closing delimiters and postfix operators cannot overflow the line.
    std::vector<std::pair<const Doc*, Mode>> fitCmd;
    size_t rest = commands.size();
    while (rem >= 0) {
        if (fitCmd.empty()) {
            if (rest == 0) {
                return true;
            }
            const auto& next = commands[--rest];
            if (rest + 1 != commands.size() && (next.first.type == DocType::SOFTLINE ||
                next.first.type == DocType::SOFTLINE_WITH_SPACE)) {
                return true;
            }
            fitCmd.emplace_back(&next.first, next.second);
        }
        auto current = fitCmd.back();
        fitCmd.pop_back();
        switch (current.first->type) {
            case DocType::STRING:
            case DocType::DOT:
                rem -= static_cast<int>(DisplayWidth(current.first->value));
                break;
            case DocType::FUNC_ARG:
            case DocType::LAMBDA:
            case DocType::ARGS:
            case DocType::CONCAT:
            case DocType::GROUP:
            case DocType::FILL:
            case DocType::MEMBER_ACCESS:
            case DocType::LAMBDA_BODY:
            case DocType::INDENT:
            case DocType::ALIGN:
                for (auto it = current.first->members.rbegin(); it != current.first->members.rend(); ++it) {
                    fitCmd.emplace_back(&*it, current.second);
                }
                break;
            case DocType::LINE:
            case DocType::HARDLINE:
            case DocType::LINE_DOT:
            case DocType::SEPARATE:
                return true;
            case DocType::SOFTLINE:
                if (current.second == Mode::MODE_BREAK) {
                    return true;
                }
                break;
            case DocType::SOFTLINE_WITH_SPACE:
                if (current.second == Mode::MODE_BREAK) {
                    return true;
                }
                rem -= 1;
                break;
            case DocType::BREAK_PARENT:
                if (current.second == Mode::MODE_BREAK) {
                    return true;
                }
                break;
            default:
                break;
        }
    }
    return false;
}

int DocProcessor::CalculateDocLen(Doc& doc)
{
    int len = 0;
    std::vector<const Doc*> pending{&doc};
    while (!pending.empty()) {
        const auto* current = pending.back();
        pending.pop_back();
        if (current->type == DocType::LINE || current->type == DocType::HARDLINE ||
            current->type == DocType::SEPARATE || current->type == DocType::LINE_DOT) {
            break;
        }
        len += current->type == DocType::SOFTLINE_WITH_SPACE ? 1 :
            static_cast<int>(DisplayWidth(current->value));
        for (auto it = current->members.rbegin(); it != current->members.rend(); ++it) {
            pending.emplace_back(&*it);
        }
    }
    return len;
}
} // namespace Cangjie::Format
