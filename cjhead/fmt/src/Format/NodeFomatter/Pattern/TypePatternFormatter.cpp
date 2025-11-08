// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Pattern/TypePatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::TypePatternFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::TYPE_PATTERN>(node);
    AddTypePattern(doc, *pattern, level);
}

void TypePatternFormatter::AddTypePattern(Doc& doc, const Cangjie::AST::TypePattern& typePattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(astToFormatSource.ASTToDoc(typePattern.pattern.get(), level));
    doc.members.emplace_back(DocType::STRING, level, ": ");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(typePattern.type.get(), level));
}
} // namespace Cangjie::Format
