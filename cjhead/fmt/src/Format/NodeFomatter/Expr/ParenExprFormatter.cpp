// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/ParenExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ParenExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto parenExpr = As<ASTKind::PAREN_EXPR>(node);
    AddParenExpr(doc, *parenExpr, level);
}
void ParenExprFormatter::AddParenExpr(Doc& doc, const Cangjie::AST::ParenExpr& parenExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (parenExpr.leftParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "(");
    }
    if (parenExpr.expr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(parenExpr.expr.get(), level));
    }
    if (parenExpr.rightParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    if (parenExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format