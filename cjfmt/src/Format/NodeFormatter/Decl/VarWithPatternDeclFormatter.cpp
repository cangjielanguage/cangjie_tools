// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/VarWithPatternDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void VarWithPatternDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto varWithPatternDecl = As<ASTKind::VAR_WITH_PATTERN_DECL>(node);
    AddVarWithPatternDecl(doc, *varWithPatternDecl, level);
}

void VarWithPatternDeclFormatter::AddVarWithPatternDecl(
    Doc& doc, const Cangjie::AST::VarWithPatternDecl& varWithPatternDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!varWithPatternDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, varWithPatternDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, varWithPatternDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level,
        varWithPatternDecl.isVar ? "var " : varWithPatternDecl.isConst ? "" : "let ");

    if (varWithPatternDecl.irrefutablePattern) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.irrefutablePattern.get(), level));
    }

    if (varWithPatternDecl.type) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.type.get(), level));
    }

    if (varWithPatternDecl.initializer) {
        doc.members.emplace_back(DocType::STRING, level, " = ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.initializer.get(), level));
    }
}
} // namespace Cangjie::Format