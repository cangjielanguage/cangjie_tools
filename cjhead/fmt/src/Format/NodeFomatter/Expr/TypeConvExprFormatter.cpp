// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/TypeConvExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void TypeConvExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto typeConvExpr = As<ASTKind::TYPE_CONV_EXPR>(node);
    AddTypeConvExpr(doc, *typeConvExpr, level);
}
void TypeConvExprFormatter::AddTypeConvExpr(Doc& doc, const Cangjie::AST::TypeConvExpr& typeConvExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(astToFormatSource.ASTToDoc(typeConvExpr.type.get(), level));
    doc.members.emplace_back(DocType::STRING, level, "(");
    if (typeConvExpr.expr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(typeConvExpr.expr.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (typeConvExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format