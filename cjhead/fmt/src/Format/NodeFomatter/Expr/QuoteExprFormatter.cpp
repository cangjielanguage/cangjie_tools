// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/QuoteExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
#include <string>

namespace Cangjie::Format {
using namespace Cangjie::AST;

void QuoteExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto quoteExpr = As<ASTKind::QUOTE_EXPR>(node);
    AddQuoteExpr(doc, *quoteExpr, level);
}
void QuoteExprFormatter::AddQuoteExpr(Doc& doc, const Cangjie::AST::QuoteExpr& quoteExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "quote(");
    std::string str = astToFormatSource.sm.GetContentBetween(
        quoteExpr.leftParenPos.fileID, quoteExpr.leftParenPos, quoteExpr.rightParenPos);
    str = str.substr(1);
    doc.members.emplace_back(DocType::STRING, level + 1, str);
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (quoteExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format