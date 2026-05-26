// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/LineDotProcessor.h"
#include <string>
#include <utility>
#include <vector>
using namespace Cangjie::Format;

void LineDotProcessor::DocToString(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    formatted += options.newLine;
    formatted += std::string(current.first.indent * options.indentWidth, ' ');
    pos = current.first.indent * options.indentWidth;
    formatted += current.first.value;
    pos += static_cast<int>(DisplayWidth(current.first.value));
}