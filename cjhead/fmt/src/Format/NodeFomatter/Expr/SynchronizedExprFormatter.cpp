// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/SynchronizedExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void SynchronizedExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto synchronizedExpr = As<ASTKind::SYNCHRONIZED_EXPR>(node);
    AddSynchronizedExpr(doc, *synchronizedExpr, level);
}
void SynchronizedExprFormatter::AddSynchronizedExpr(
    Doc& doc, const Cangjie::AST::SynchronizedExpr& synchronizedExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (synchronizedExpr.syncPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "synchronized");
    }
    if (synchronizedExpr.leftParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "(");
    }
    if (synchronizedExpr.mutex) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(synchronizedExpr.mutex.get(), level + 1));
    }
    if (synchronizedExpr.rightParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    if (synchronizedExpr.body) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(synchronizedExpr.body.get(), level));
    }
}
} // namespace Cangjie::Format