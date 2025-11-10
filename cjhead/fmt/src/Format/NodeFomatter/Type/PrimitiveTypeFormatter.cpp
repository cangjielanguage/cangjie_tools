// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/PrimitiveTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::PrimitiveTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::PRIMITIVE_TYPE>(node);
    AddPrimitiveType(doc, *type, level);
}

void PrimitiveTypeFormatter::AddPrimitiveType(Doc& doc, const Cangjie::AST::PrimitiveType& primitiveType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    Doc group(DocType::GROUP, level, "");
    if (!primitiveType.GetTypeParameterNameRawText().empty()) {
        group.members.emplace_back(DocType::STRING, level, primitiveType.GetTypeParameterNameRawText());
        group.members.emplace_back(DocType::STRING, level, ": ");
    }
    group.members.emplace_back(DocType::STRING, level, primitiveType.str);
    doc.members.emplace_back(group);
}
} // namespace Cangjie::Format
