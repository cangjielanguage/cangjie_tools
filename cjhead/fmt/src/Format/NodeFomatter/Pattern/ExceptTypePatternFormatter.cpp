// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Pattern/ExceptTypePatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::ExceptTypePatternFormatter::ASTToDoc(
    Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::EXCEPT_TYPE_PATTERN>(node);
    AddExceptTypePattern(doc, *pattern, level);
}

void ExceptTypePatternFormatter::AddExceptTypePattern(
    Doc& doc, const Cangjie::AST::ExceptTypePattern& exceptTypePattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (exceptTypePattern.pattern) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(exceptTypePattern.pattern.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, ": ");
    for (auto& type : exceptTypePattern.types) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(type.get(), level));
        if (type != exceptTypePattern.types.back()) {
            doc.members.emplace_back(DocType::STRING, level, " | ");
        }
    }
}
} // namespace Cangjie::Format
