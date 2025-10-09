// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/InterfaceDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void InterfaceDeclFormatter::AddInterfaceDecl(Doc& doc, const Cangjie::AST::InterfaceDecl& interfaceDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!interfaceDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, interfaceDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, interfaceDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "interface ");
    doc.members.emplace_back(DocType::STRING, level, interfaceDecl.identifier.GetRawText());
    auto& generic = interfaceDecl.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    if (!interfaceDecl.inheritedTypes.empty()) {
        AddInterfaceInheritedTypes(doc, interfaceDecl, level);
    }
    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(interfaceDecl.body.get(), level));
}

void InterfaceDeclFormatter::AddInterfaceInheritedTypes(
    Doc& doc, const Cangjie::AST::InterfaceDecl& interfaceDecl, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& type : interfaceDecl.inheritedTypes) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(type.get(), level + 1));
        if (type != interfaceDecl.inheritedTypes.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            group.members.emplace_back(DocType::STRING, level + 1, "&");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}

void InterfaceDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto interfaceDecl = As<ASTKind::INTERFACE_DECL>(node);
    AddInterfaceDecl(doc, *interfaceDecl, level);
}
} // namespace Cangjie::Format