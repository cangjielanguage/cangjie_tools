// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/DotProcessor.h"
#include <string>
#include <utility>
#include <vector>

using namespace Cangjie::Format;

void DotProcessor::DocToString(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    formatted += current.first.value;
    pos += static_cast<int>(DisplayWidth(current.first.value));
}