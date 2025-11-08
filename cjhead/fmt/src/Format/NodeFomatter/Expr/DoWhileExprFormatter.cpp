// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/DoWhileExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void DoWhileExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto doWhileExpr = StaticAs<ASTKind::DO_WHILE_EXPR>(node);
    AddDoWhileExpr(doc, *doWhileExpr, level);
}
void DoWhileExprFormatter::AddDoWhileExpr(Doc& doc, const Cangjie::AST::DoWhileExpr& doWhileExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "do");
    if (doWhileExpr.body) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(doWhileExpr.body.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " while ");
    if (doWhileExpr.leftParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "(");
    }
    // body
    if (doWhileExpr.condExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(doWhileExpr.condExpr.get(), level));
    }
    if (doWhileExpr.rightParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    if (doWhileExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format