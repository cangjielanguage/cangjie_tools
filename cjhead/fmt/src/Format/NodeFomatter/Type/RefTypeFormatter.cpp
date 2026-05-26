// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/RefTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::RefTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::REF_TYPE>(node);
    AddRefType(doc, *type, level);
}

void RefTypeFormatter::AddRefType(Doc& doc, const Cangjie::AST::RefType& refType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!refType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, refType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    doc.members.emplace_back(DocType::STRING, level, refType.ref.identifier.GetRawText());
    if (!refType.typeArguments.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "<");
        for (auto& arg : refType.typeArguments) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(arg.get(), level));
            if (arg != refType.typeArguments.back()) {
                doc.members.emplace_back(DocType::STRING, level, ",");
                doc.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, ">");
    }
}
} // namespace Cangjie::Format
