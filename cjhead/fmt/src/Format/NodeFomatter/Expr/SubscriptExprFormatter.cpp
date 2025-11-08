// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/SubscriptExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void SubscriptExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto subscriptExpr = StaticAs<ASTKind::SUBSCRIPT_EXPR>(node);
    AddSubscriptExpr(doc, *subscriptExpr, level, funcOptions);
}
void SubscriptExprFormatter::AddSubscriptExpr(
    Doc& doc, const Cangjie::AST::SubscriptExpr& subscriptExpr, int level, FuncOptions& funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (subscriptExpr.baseExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(subscriptExpr.baseExpr.get(), level, funcOptions));
    }
    doc.members.emplace_back(DocType::STRING, level, "[");
    for (auto& n : subscriptExpr.indexExprs) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level + 1));
        if (n != subscriptExpr.indexExprs.back()) {
            doc.members.emplace_back(DocType::STRING, level + 1, ",");
            doc.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(DocType::STRING, level, "]");
    if (subscriptExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format
