// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/ThrowExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ThrowExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto* throwExpr = As<ASTKind::THROW_EXPR>(node);
    AddThrowExpr(doc, *throwExpr, level);
}
void ThrowExprFormatter::AddThrowExpr(Doc& doc, const Cangjie::AST::ThrowExpr& throwExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "throw ");
    if (throwExpr.expr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(throwExpr.expr.get(), level));
    }
}
} // namespace Cangjie::Format