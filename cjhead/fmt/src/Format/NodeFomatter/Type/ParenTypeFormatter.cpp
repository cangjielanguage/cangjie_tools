// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/ParenTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::ParenTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::PAREN_TYPE>(node);
    AddParenType(doc, *type, level);
}

void ParenTypeFormatter::AddParenType(Doc& doc, const Cangjie::AST::ParenType& parenType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!parenType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, parenType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    doc.members.emplace_back(DocType::STRING, level, "(");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(parenType.type.get(), level + 1));
    doc.members.emplace_back(DocType::STRING, level, ")");
}
} // namespace Cangjie::Format
