// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Pattern/EnumPatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void EnumPatternFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::ENUM_PATTERN>(node);
    AddEnumPattern(doc, *pattern, level);
}

void EnumPatternFormatter::AddEnumPattern(Doc& doc, const Cangjie::AST::EnumPattern& enumPattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (enumPattern.constructor) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(enumPattern.constructor.get(), level));
    }
    if (!enumPattern.patterns.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "(");
        for (auto& par : enumPattern.patterns) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(par.get(), level + 1));
            if (par != enumPattern.patterns.back()) {
                doc.members.emplace_back(DocType::STRING, level + 1, ", ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
}
} // namespace Cangjie::Format