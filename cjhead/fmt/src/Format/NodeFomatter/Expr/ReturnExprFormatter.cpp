// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/ReturnExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ReturnExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto returnExpr = As<ASTKind::RETURN_EXPR>(node);
    AddReturnExpr(doc, *returnExpr, level);
}
void ReturnExprFormatter::AddReturnExpr(Doc& doc, const Cangjie::AST::ReturnExpr& returnExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!returnExpr.TestAttr(Attribute::COMPILER_ADD)) {
        doc.members.emplace_back(DocType::STRING, level, "return");
    }
    if (returnExpr.expr && !returnExpr.TestAttr(Attribute::COMPILER_ADD) &&
        !returnExpr.expr->TestAttr(Attribute::COMPILER_ADD)) {
        doc.members.emplace_back(DocType::STRING, level, " ");
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(returnExpr.expr.get(), level));
    if (returnExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format