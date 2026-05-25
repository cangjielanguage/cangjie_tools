// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Pattern/TuplePatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::TuplePatternFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::TUPLE_PATTERN>(node);
    AddTuplePattern(doc, *pattern, level);
}

void TuplePatternFormatter::AddTuplePattern(Doc& doc, const Cangjie::AST::TuplePattern& tuplePattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "(");
    for (auto& par : tuplePattern.patterns) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(par.get(), level + 1));
        if (par != tuplePattern.patterns.back()) {
            doc.members.emplace_back(DocType::STRING, level + 1, ", ");
        }
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
}
} // namespace Cangjie::Format
