// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/OptionTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::OptionTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::OPTION_TYPE>(node);
    AddOptionType(doc, *type, level);
}

void OptionTypeFormatter::AddOptionType(Doc& doc, const Cangjie::AST::OptionType& optionType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!optionType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, optionType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    for (size_t i = 0; i < optionType.questVector.size(); ++i) {
        doc.members.emplace_back(DocType::STRING, level, "?");
    }
    if (optionType.componentType) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(optionType.componentType.get(), level));
    }
}
} // namespace Cangjie::Format
