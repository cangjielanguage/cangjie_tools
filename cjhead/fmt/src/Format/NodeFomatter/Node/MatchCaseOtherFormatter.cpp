// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/MatchCaseOtherFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::MatchCaseOtherFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto matchCaseOther = As<ASTKind::MATCH_CASE_OTHER>(node);
    AddMatchCaseOther(doc, *matchCaseOther, level);
}
void MatchCaseOtherFormatter::AddMatchCaseOther(Doc& doc, const Cangjie::AST::MatchCaseOther& matchCaseOther, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "case ");
    if (matchCaseOther.matchExpr) {
        FuncOptions funcOptions;
        funcOptions.patternOrEnum = true;
        doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCaseOther.matchExpr.get(), level, funcOptions));
    }
    doc.members.emplace_back(DocType::STRING, level, " =>");
    if (matchCaseOther.exprOrDecls) {
        if (matchCaseOther.exprOrDecls->body.size() == 1) {
            doc.members.emplace_back(DocType::STRING, level, " ");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCaseOther.exprOrDecls.get(), level));
        } else {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCaseOther.exprOrDecls.get(), level + 1));
        }
    }
}
} // namespace Cangjie::Format
