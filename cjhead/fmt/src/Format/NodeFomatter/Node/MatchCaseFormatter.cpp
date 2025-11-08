// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/MatchCaseFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::MatchCaseFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto matchCase = As<ASTKind::MATCH_CASE>(node);
    AddMatchCase(doc, *matchCase, level);
}
void MatchCaseFormatter::AddMatchCase(Doc& doc, const Cangjie::AST::MatchCase& matchCase, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "case ");
    for (auto& expr : matchCase.patterns) {
        if (expr != matchCase.patterns.front()) {
            doc.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            Doc group(DocType::GROUP, level, "");
            group.members.emplace_back(DocType::STRING, level, "| ");
            group.members.emplace_back(astToFormatSource.ASTToDoc(expr.get(), level + 1));
            doc.members.emplace_back(group);
        } else {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(expr.get(), level + 1));
        }
    }
    if (matchCase.patternGuard) {
        doc.members.emplace_back(DocType::STRING, level, " where ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCase.patternGuard.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " =>");
    if (matchCase.exprOrDecls) {
        if (matchCase.exprOrDecls->body.size() == 1 && matchCase.exprOrDecls->body[0]->astKind != ASTKind::IF_EXPR) {
            doc.members.emplace_back(DocType::STRING, level, " ");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCase.exprOrDecls.get(), level));
        } else {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCase.exprOrDecls.get(), level + 1));
        }
    }
}
} // namespace Cangjie::Format
