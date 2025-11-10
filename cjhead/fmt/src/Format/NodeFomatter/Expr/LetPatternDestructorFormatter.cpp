// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/LetPatternDestructorFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void LetPatternDestructorFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto letPatternDestructor = As<ASTKind::LET_PATTERN_DESTRUCTOR>(node);
    AddLetPatternDestructor(doc, *letPatternDestructor, level);
}
void LetPatternDestructorFormatter::AddLetPatternDestructor(
    Doc& doc, const Cangjie::AST::LetPatternDestructor& letPatternDestructor, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "let ");
    for (auto& pattern : letPatternDestructor.patterns) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(pattern.get(), level));
        if (pattern != letPatternDestructor.patterns.back()) {
            doc.members.emplace_back(DocType::STRING, level, " | ");
        }
    }
    if (letPatternDestructor.initializer) {
        doc.members.emplace_back(DocType::STRING, level, " <- ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(letPatternDestructor.initializer.get(), level));
    }
}
} // namespace Cangjie::Format