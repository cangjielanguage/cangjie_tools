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
    std::vector<std::pair<Doc, Mode>> fitCmd;
    fitCmd.emplace_back(next);
    while (rem >= 0) {
        if (fitCmd.empty()) {
            return true;
        }
        std::pair<Doc, Mode> current = fitCmd.back();
        fitCmd.pop_back();
        switch (current.first.type) {
            case DocType::STRING:
                rem -= static_cast<int>(current.first.value.length());
                return rem >= 0;
            case DocType::FUNC_ARG:
            case DocType::LAMBDA:
            case DocType::CONCAT:
                for (auto it = current.first.members.rbegin(); it != current.first.members.rend(); ++it) {
                    fitCmd.emplace_back(*it, Mode::MODE_FLAT);
                }
                break;
            case DocType::LINE:
            case DocType::SEPARATE:
                return true;
            case DocType::GROUP:
                return rem - CalculateDocLen(current.first) > 0;
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
    len += static_cast<int>(DisplayWidth(doc.value));
    for (auto& member : doc.members) {
        if (member.type == DocType::LINE) {
            return len;
        }
        len += CalculateDocLen(member);
    }
    return len;
}
} // namespace Cangjie::Format
