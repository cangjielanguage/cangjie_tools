// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/RangeExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void RangeExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto rangeExpr = As<ASTKind::RANGE_EXPR>(node);
    AddRangeExpr(doc, *rangeExpr, level);
}
void RangeExprFormatter::AddRangeExpr(Doc& doc, const Cangjie::AST::RangeExpr& rangeExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (rangeExpr.startExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(rangeExpr.startExpr.get(), level));
    }
    if (rangeExpr.isClosed) {
        doc.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(TokenKind::CLOSEDRANGEOP)]);
    } else {
        doc.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(TokenKind::RANGEOP)]);
    }
    if (rangeExpr.stopExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(rangeExpr.stopExpr.get(), level));
    }
    if (rangeExpr.stepExpr) {
        doc.members.emplace_back(DocType::STRING, level, " : ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(rangeExpr.stepExpr.get(), level));
    }
}
} // namespace Cangjie::Format