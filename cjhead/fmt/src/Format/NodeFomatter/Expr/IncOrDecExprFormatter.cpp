// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/IncOrDecExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void IncOrDecExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto incOrDecExpr = As<ASTKind::INC_OR_DEC_EXPR>(node);
    AddIncOrDecExpr(doc, *incOrDecExpr, level);
}
void IncOrDecExprFormatter::AddIncOrDecExpr(Doc& doc, const Cangjie::AST::IncOrDecExpr& incOrDecExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (incOrDecExpr.expr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(incOrDecExpr.expr.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(incOrDecExpr.op)]);
    if (incOrDecExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format