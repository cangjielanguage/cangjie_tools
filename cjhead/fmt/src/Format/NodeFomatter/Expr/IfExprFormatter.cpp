// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/IfExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void IfExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto ifExpr = As<ASTKind::IF_EXPR>(node);
    AddIfExpr(doc, *ifExpr, level);
}
void IfExprFormatter::AddIfExpr(Doc& doc, const Cangjie::AST::IfExpr& ifExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "if ");

    if (ifExpr.condExpr) {
        doc.members.emplace_back(DocType::STRING, level, "(");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(ifExpr.condExpr.get(), level));
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(ifExpr.thenBody.get(), level));
    if (ifExpr.hasElse) {
        ifExpr.thenBody->body.empty() && ifExpr.thenBody->leftCurlPos.line == ifExpr.thenBody->rightCurlPos.line
            ? doc.members.emplace_back(DocType::LINE, level, "")
            : doc.members.emplace_back(DocType::STRING, level, " ");
        doc.members.emplace_back(DocType::STRING, level, "else");
        if (ifExpr.elseBody->astKind == ASTKind::IF_EXPR) {
            doc.members.emplace_back(DocType::STRING, level, " ");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(ifExpr.elseBody.get(), level));
    }
}
} // namespace Cangjie::Format