// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/AsExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void AsExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto asExpr = As<ASTKind::AS_EXPR>(node);
    AddAsExpr(doc, *asExpr, level);
}
void AsExprFormatter::AddAsExpr(Doc& doc, const Cangjie::AST::AsExpr& asExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (asExpr.leftExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(asExpr.leftExpr.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " as ");
    if (asExpr.asType) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(asExpr.asType.get(), level));
    }
}
} // namespace Cangjie::Format