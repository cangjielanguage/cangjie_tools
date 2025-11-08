// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/ValWithGradExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ValWithGradExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto expr = As<ASTKind::VALWITHGRAD_EXPR>(node);
    AddValWithGradExpr(doc, *expr, level);
}
void ValWithGradExprFormatter::AddValWithGradExpr(
    Doc& doc, const Cangjie::AST::ValWithGradExpr& valWithGradExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "@ValWithGrad(");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(valWithGradExpr.f.get(), level));
    for (auto& item : valWithGradExpr.inputVector) {
        doc.members.emplace_back(DocType::STRING, level, ", ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(item.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (valWithGradExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format