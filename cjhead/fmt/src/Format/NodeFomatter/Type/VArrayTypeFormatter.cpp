// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/VArrayTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::VArrayTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::VARRAY_TYPE>(node);
    AddVArrayType(doc, *type, level);
}

void VArrayTypeFormatter::AddVArrayType(Doc& doc, const Cangjie::AST::VArrayType& vArrayType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!vArrayType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, vArrayType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    doc.members.emplace_back(DocType::STRING, level, "VArray");
    doc.members.emplace_back(DocType::STRING, level, "<");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(vArrayType.typeArgument.get(), level));
    doc.members.emplace_back(DocType::STRING, level, ", ");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(vArrayType.constantType.get(), level));
    doc.members.emplace_back(DocType::STRING, level, ">");
}
} // namespace Cangjie::Format