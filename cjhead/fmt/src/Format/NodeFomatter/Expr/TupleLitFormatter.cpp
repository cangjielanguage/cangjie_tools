// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/TupleLitFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void TupleLitFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto tupleLit = As<ASTKind::TUPLE_LIT>(node);
    AddTupleLit(doc, *tupleLit, level);
}
void TupleLitFormatter::AddTupleLit(Doc& doc, const Cangjie::AST::TupleLit& tupleLit, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "(");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& expr : tupleLit.children) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(expr.get(), level + 1));
        if (expr != tupleLit.children.back()) {
            group.members.emplace_back(DocType::STRING, level + 1, ",");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (tupleLit.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format