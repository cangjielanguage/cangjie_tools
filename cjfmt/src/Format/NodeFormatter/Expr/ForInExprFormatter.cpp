// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Expr/ForInExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void ForInExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto forInExpr = As<ASTKind::FOR_IN_EXPR>(node);
    AddForInExpr(doc, *forInExpr, level);
}

void ForInExprFormatter::AddForInExpr(Doc& doc, const Cangjie::AST::ForInExpr& forInExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "for ");
    if (forInExpr.leftParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "(");
    }
    if (forInExpr.pattern) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(forInExpr.pattern.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " in ");
    if (forInExpr.inExpression) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(forInExpr.inExpression.get(), level));
    }
    if (forInExpr.patternGuard) {
        doc.members.emplace_back(DocType::STRING, level, " where ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(forInExpr.patternGuard.get(), level));
    }
    if (forInExpr.rightParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    if (forInExpr.body) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(forInExpr.body.get(), level));
    }
}
} // namespace Cangjie::Format