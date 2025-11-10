// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/SeparateProcessor.h"
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"
#include <string>
#include <utility>
#include <vector>

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::SeparateProcessor::DocToString(std::string &formatted, int &pos, std::pair<Doc, Mode> &,
    std::vector<std::pair<Doc, Mode>> &)
{
    formatted += options.newLine;
    pos = 0;
}
}
