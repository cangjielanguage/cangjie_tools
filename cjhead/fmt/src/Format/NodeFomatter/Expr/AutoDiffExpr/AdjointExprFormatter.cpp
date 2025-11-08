// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/AdjointExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void AdjointExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto expr = As<ASTKind::ADJOINT_EXPR>(node);
    AddAdjointExpr(doc, *expr, level);
}
void AdjointExprFormatter::AddAdjointExpr(Doc& doc, const Cangjie::AST::AdjointExpr& adjointExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "@AdjointOf(");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(adjointExpr.f.get(), level));
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (adjointExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format