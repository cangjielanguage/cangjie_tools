// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/StructDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void StructDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto structDecl = As<ASTKind::STRUCT_DECL>(node);
    AddStructDecl(doc, *structDecl, level);
}

void StructDeclFormatter::AddStructDecl(Doc& doc, const Cangjie::AST::StructDecl& structDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!structDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, structDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, structDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "struct ");
    doc.members.emplace_back(DocType::STRING, level, structDecl.identifier.GetRawText());
    auto& generic = structDecl.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    if (!structDecl.inheritedTypes.empty() && structDecl.upperBoundPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, " <: ");
        for (const auto& ty : structDecl.inheritedTypes) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(ty.get(), level));
            if (ty != structDecl.inheritedTypes.back()) {
                doc.members.emplace_back(DocType::STRING, level, " & ");
            }
        }
    }
    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(structDecl.body.get(), level));
}
} // namespace Cangjie::Format