// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Pattern/ConstPatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::ConstPatternFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::CONST_PATTERN>(node);
    AddConstPattern(doc, *pattern, level);
}

void ConstPatternFormatter::AddConstPattern(Doc& doc, const Cangjie::AST::ConstPattern& constPattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(astToFormatSource.ASTToDoc(constPattern.literal.get(), level + 1));
}
} // namespace Cangjie::Format
