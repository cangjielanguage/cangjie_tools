// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/BinaryExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void BinaryExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto binaryExpr = As<ASTKind::BINARY_EXPR>(node);
    AddBinaryExpr(doc, *binaryExpr, level);
}
void BinaryExprFormatter::AddBinaryExpr(Doc& doc, const Cangjie::AST::BinaryExpr& binaryExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    Doc group(DocType::GROUP, level, "");
    group.members.emplace_back(astToFormatSource.ASTToDoc(binaryExpr.leftExpr.get(), level));
    binaryExpr.leftExpr->end.line < binaryExpr.operatorPos.line
        ? group.members.emplace_back(DocType::LINE, level + 1, "")
        : group.members.emplace_back(DocType::STRING, level, " ");
    group.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(binaryExpr.op)]);
    group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
    doc.members.emplace_back(group);
    doc.members.emplace_back(astToFormatSource.ASTToDoc(binaryExpr.rightExpr.get(), level));
    if (binaryExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format