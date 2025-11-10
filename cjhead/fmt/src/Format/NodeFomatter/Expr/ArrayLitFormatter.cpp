// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/ArrayLitFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ArrayLitFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto al = As<ASTKind::ARRAY_LIT>(node);
    AddArrayLit(doc, *al, level);
}
void ArrayLitFormatter::AddArrayLit(Doc& doc, const Cangjie::AST::ArrayLit& arrayLit, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "[");
    if (astToFormatSource.IsMultipleLineArrayLit(arrayLit.rightSquarePos.line, arrayLit.children) ||
        astToFormatSource.IsMultipleLineExpr(arrayLit.children)) {
        AddBreakLineArrayLit(doc, arrayLit, level);
        return;
    }
    if (!arrayLit.children.empty()) {
        AddArrayLitChildren(doc, arrayLit, level);
    }
    doc.members.emplace_back(DocType::STRING, level, "]");
    if (arrayLit.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
void ArrayLitFormatter::AddBreakLineArrayLit(Doc& doc, const Cangjie::AST::ArrayLit& arrayLit, int level)
{
    for (auto it = arrayLit.children.begin(); it != arrayLit.children.end(); ++it) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(it->get(), level + 1));
        if (*it != arrayLit.children.back()) {
            doc.members.emplace_back(DocType::STRING, level + 1, ",");
        }
    }
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "]");
    if (arrayLit.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
void ArrayLitFormatter::AddArrayLitChildren(Doc& doc, const Cangjie::AST::ArrayLit& arrayLit, int level)
{
    Doc args(DocType::ARGS, level, "");
    for (auto& n : arrayLit.children) {
        Doc group(DocType::GROUP, level, "");
        group.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level));
        args.members.emplace_back(group);
        if (n != arrayLit.children.back()) {
            args.members.emplace_back(DocType::STRING, level, ",");
            args.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(args);
    doc.members.emplace_back(DocType::BREAK_PARENT, level, "");
}
} // namespace Cangjie::Format