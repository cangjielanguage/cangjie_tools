// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/TypeAliasDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void TypeAliasDeclFormatter::AddTypeAliasDecl(Doc& doc, const Cangjie::AST::TypeAliasDecl& typeAliasDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!typeAliasDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, typeAliasDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, typeAliasDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "type ");
    doc.members.emplace_back(DocType::STRING, level, typeAliasDecl.identifier.GetRawText());
    auto& generic = typeAliasDecl.generic;
    if (typeAliasDecl.generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(DocType::STRING, level, " = ");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(typeAliasDecl.type.get(), level));
}

void TypeAliasDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto decl = As<ASTKind::TYPE_ALIAS_DECL>(node);
    AddTypeAliasDecl(doc, *decl, level);
}
} // namespace Cangjie::Format
