// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/WhileExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void WhileExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto whileExpr = As<ASTKind::WHILE_EXPR>(node);
    AddWhileExpr(doc, *whileExpr, level);
}
void WhileExprFormatter::AddWhileExpr(Doc& doc, const Cangjie::AST::WhileExpr& whileExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "while ");
    if (whileExpr.leftParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "(");
    }
    if (whileExpr.condExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(whileExpr.condExpr.get(), level + 1));
    }
    if (whileExpr.rightParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    if (whileExpr.body) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(whileExpr.body.get(), level));
    }
}
} // namespace Cangjie::Format