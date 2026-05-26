// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/UnaryExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::UnaryExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto unaryExpr = As<ASTKind::UNARY_EXPR>(node);
    AddUnaryExpr(doc, *unaryExpr, level);
}
void UnaryExprFormatter::AddUnaryExpr(Doc& doc, const Cangjie::AST::UnaryExpr& unaryExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    Doc group(DocType::GROUP, level, "");
    group.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(unaryExpr.op)]);

    if (unaryExpr.op == TokenKind::SUB &&
        (unaryExpr.expr.get()->astKind == ASTKind::LIT_CONST_EXPR ||
            unaryExpr.expr.get()->astKind == ASTKind::UNARY_EXPR)) {
        group.members.emplace_back(DocType::STRING, level, " ");
    }
    group.members.emplace_back(astToFormatSource.ASTToDoc(unaryExpr.expr.get(), level));
    doc.members.emplace_back(group);
}
} // namespace Cangjie::Format
