// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/ClassDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ClassDeclFormatter::AddClassDeclInheritedTypes(Doc& doc, const Cangjie::AST::ClassDecl& classDecl, int level)
{
    if (!classDecl.inheritedTypes.empty() && classDecl.upperBoundPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, " <: ");
        for (const auto& ty : classDecl.inheritedTypes) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(ty.get(), level));
            if (ty != classDecl.inheritedTypes.back()) {
                doc.members.emplace_back(DocType::STRING, level, " & ");
            }
        }
    }
}

void ClassDeclFormatter::AddClassDecl(Doc& doc, const Cangjie::AST::ClassDecl& classDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!classDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, classDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, classDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "class " + classDecl.identifier.GetRawText());
    auto& generic = classDecl.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    AddClassDeclInheritedTypes(doc, classDecl, level);
    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(classDecl.body.get(), level));
}

void ClassDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto classDecl = As<ASTKind::CLASS_DECL>(node);
    AddClassDecl(doc, *classDecl, level);
}
} // namespace Cangjie::Format