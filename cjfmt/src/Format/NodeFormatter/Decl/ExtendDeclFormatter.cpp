// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/ExtendDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ExtendDeclFormatter::AddExtendDecl(Doc& doc, const Cangjie::AST::ExtendDecl& extendDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!extendDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, extendDecl.annotations, level);
    }

    if (extendDecl.TestAttr(AST::Attribute::COMMON)) {
        doc.members.emplace_back(DocType::STRING, level, "common");
        doc.members.emplace_back(DocType::STRING, level, " ");
    }

    if (extendDecl.TestAttr(AST::Attribute::PLATFORM)) {
        doc.members.emplace_back(DocType::STRING, level, "platform");
        doc.members.emplace_back(DocType::STRING, level, " ");
    }

    doc.members.emplace_back(DocType::STRING, level, "extend");
    if (extendDecl.generic) {
        astToFormatSource.AddGenericParams(doc, *extendDecl.generic, level);
    }
    doc.members.emplace_back(DocType::STRING, level, " ");

    doc.members.emplace_back(astToFormatSource.ASTToDoc(extendDecl.extendedType.get(), level));
    if (!extendDecl.inheritedTypes.empty()) {
        AddExtendDeclInheritedTypes(doc, extendDecl, level);
    }
    if (extendDecl.generic) {
        astToFormatSource.AddGenericBound(doc, *extendDecl.generic, level);
    }

    // Body
    if (extendDecl.members.empty()) {
        doc.members.emplace_back(DocType::STRING, level, " {}");
        return;
    }
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    astToFormatSource.AddBodyMembers(doc, extendDecl.members, level + 1);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void ExtendDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto extendDecl = As<ASTKind::EXTEND_DECL>(node);
    AddExtendDecl(doc, *extendDecl, level);
}

void ExtendDeclFormatter::AddExtendDeclInheritedTypes(Doc& doc, const Cangjie::AST::ExtendDecl& extendDecl, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& trait : extendDecl.inheritedTypes) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(trait.get(), level + 1));
        if (trait != extendDecl.inheritedTypes.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            group.members.emplace_back(DocType::STRING, level + 1, "&");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}
} // namespace Cangjie::Format
