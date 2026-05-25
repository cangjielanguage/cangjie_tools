// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/TupleTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::TupleTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::TUPLE_TYPE>(node);
    AddTupleType(doc, *type, level);
}

void TupleTypeFormatter::AddTupleType(Doc& doc, const Cangjie::AST::TupleType& tupleType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!tupleType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, tupleType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    doc.members.emplace_back(DocType::STRING, level, "(");
    for (auto& field : tupleType.fieldTypes) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(field.get(), level));
        if (field != tupleType.fieldTypes.back()) {
            doc.members.emplace_back(DocType::STRING, level, ", ");
            doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
        }
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
}
} // namespace Cangjie::Format
