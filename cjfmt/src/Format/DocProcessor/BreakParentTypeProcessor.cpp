// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/DocProcessor/BreakParentTypeProcessor.h"
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::BreakParentTypeProcessor::DocToString(std::string &, int &,
    std::pair<Doc, Mode> &, std::vector<std::pair<Doc, Mode>> &)
{
    // Layout marker only. MeasureFlat rejects it, forcing the enclosing semantic Group to break.
}
}
