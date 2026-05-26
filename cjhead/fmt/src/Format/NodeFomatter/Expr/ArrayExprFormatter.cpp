// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/ArrayExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ArrayExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto arrayExpr = As<ASTKind::ARRAY_EXPR>(node);
    AddArrayExpr(doc, *arrayExpr, level);
}
void ArrayExprFormatter::AddArrayExpr(Doc& doc, const Cangjie::AST::ArrayExpr& arrayExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (arrayExpr.type) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(arrayExpr.type.get(), level));
    }
    if (arrayExpr.leftParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "(");
    }
    for (auto& arg : arrayExpr.args) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(arg.get(), level + 1));
        if (arg != arrayExpr.args.back()) {
            doc.members.emplace_back(DocType::STRING, level + 1, ", ");
        }
    }
    if (arrayExpr.rightParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    if (arrayExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format