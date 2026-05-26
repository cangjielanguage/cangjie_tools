// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/AssignExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void AssignExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto assignExpr = As<ASTKind::ASSIGN_EXPR>(node);
    AddAssignExpr(doc, *assignExpr, level);
}
void AssignExprFormatter::AddAssignExpr(Doc& doc, const Cangjie::AST::AssignExpr& assignExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(astToFormatSource.ASTToDoc(assignExpr.leftValue.get(), level));
    doc.members.emplace_back(DocType::STRING, level, " ");
    doc.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(assignExpr.op)]);
    doc.members.emplace_back(DocType::STRING, level, " ");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(assignExpr.rightExpr.get(), level));
}
} // namespace Cangjie::Format