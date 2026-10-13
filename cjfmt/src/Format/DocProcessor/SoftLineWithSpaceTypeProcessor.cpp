// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/SoftLineWithSpaceTypeProcessor.h"
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::SoftLineWithSpaceTypeProcessor::DocToString(std::string &formatted, int &pos,
    std::pair<Doc, Mode> &current, std::vector<std::pair<Doc, Mode>> &leftCmd)
{
    if (leftCmd.empty()) {
        return;
    }
    const auto& next = leftCmd.back();
    bool addSpace = next.first.type != DocType::LINE && next.first.type != DocType::HARDLINE;
    auto rem = options.lineLength - pos - (addSpace ? 1 : 0);
    if (!Fits(leftCmd, rem)) {
        formatted += options.newLine;
        formatted += std::string(current.first.indent * options.indentWidth, ' ');
        pos = current.first.indent * options.indentWidth;
    } else {
        if (addSpace) {
            formatted += " ";
            pos += 1;
        }
    }
}
}
