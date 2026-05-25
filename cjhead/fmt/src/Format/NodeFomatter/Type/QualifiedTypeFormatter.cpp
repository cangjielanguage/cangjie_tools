// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/QualifiedTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::QualifiedTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto qualifiedType = As<ASTKind::QUALIFIED_TYPE>(node);
    AddQualifiedType(doc, *qualifiedType, level);
}

void QualifiedTypeFormatter::AddQualifiedType(Doc& doc, const QualifiedType& qualifiedType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!qualifiedType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, qualifiedType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    if (qualifiedType.baseType) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(qualifiedType.baseType.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, ".");
    doc.members.emplace_back(DocType::STRING, level, qualifiedType.field);
    if (qualifiedType.leftAnglePos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "<");
        for (auto& argument : qualifiedType.typeArguments) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(argument.get(), level));
            if (argument != qualifiedType.typeArguments.back()) {
                doc.members.emplace_back(DocType::STRING, level, ", ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, ">");
    }
}
} // namespace Cangjie::Format
