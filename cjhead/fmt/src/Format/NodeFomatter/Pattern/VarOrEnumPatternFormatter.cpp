// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Pattern/VarOrEnumPatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void VarOrEnumPatternFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::VAR_OR_ENUM_PATTERN>(node);
    AddVarOrEnumPattern(doc, *pattern, level);
}

void VarOrEnumPatternFormatter::AddVarOrEnumPattern(
    Doc& doc, const Cangjie::AST::VarOrEnumPattern& varOrEnumPattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, varOrEnumPattern.identifier.GetRawText());
}
} // namespace Cangjie::Format