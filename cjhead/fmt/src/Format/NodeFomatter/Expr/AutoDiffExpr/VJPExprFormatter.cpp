// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/VJPExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void VJPExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto expr = As<ASTKind::VJP_EXPR>(node);
    AddVJPExpr(doc, *expr, level);
}
void VJPExprFormatter::AddVJPExpr(Doc& doc, const Cangjie::AST::VJPExpr& vjpExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "@VJP(");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(vjpExpr.diffFunc.get(), level));
    for (auto& item : vjpExpr.inputVector) {
        doc.members.emplace_back(DocType::STRING, level, ", ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(item.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (vjpExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format