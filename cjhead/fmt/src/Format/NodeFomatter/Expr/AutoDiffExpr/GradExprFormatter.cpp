// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/GradExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void GradExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto expr = As<ASTKind::GRAD_EXPR>(node);
    AddGradExpr(doc, *expr, level);
}
void GradExprFormatter::AddGradExpr(Doc& doc, const Cangjie::AST::GradExpr& gradExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "@Grad(");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(gradExpr.f.get(), level));
    for (auto& item : gradExpr.inputVector) {
        doc.members.emplace_back(DocType::STRING, level, ", ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(item.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (gradExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format