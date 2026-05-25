// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/MatchExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void MatchExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto matchExpr = As<ASTKind::MATCH_EXPR>(node);
    AddMatchExpr(doc, *matchExpr, level);
}
void MatchExprFormatter::AddMatchExpr(Doc& doc, const Cangjie::AST::MatchExpr& matchExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "match ");
    // select
    astToFormatSource.AddMatchSelector(doc, matchExpr, level);
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    // MatchCase
    astToFormatSource.AddBodyMembers(doc, matchExpr.matchCases, level + 1);
    // MatchCaseOther
    astToFormatSource.AddBodyMembers(doc, matchExpr.matchCaseOthers, level + 1);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
    if (matchExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format