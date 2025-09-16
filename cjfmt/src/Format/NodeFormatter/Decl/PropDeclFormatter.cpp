// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/PropDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void PropDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto propDecl = As<ASTKind::PROP_DECL>(node);
    AddPropDecl(doc, *propDecl, level);
}

void PropDeclFormatter::AddPropDecl(Doc& doc, const Cangjie::AST::PropDecl& propDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!propDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, propDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, propDecl.modifiers, level);

    doc.members.emplace_back(DocType::STRING, level, "prop ");
    doc.members.emplace_back(DocType::STRING, level, propDecl.identifier.GetRawText());
    if (propDecl.type) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(propDecl.type.get(), level));
    }

    if (propDecl.leftCurlPos != INVALID_POSITION && propDecl.rightCurlPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, " {");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        astToFormatSource.AddBodyMembers(doc, propDecl.getters, level + 1);
        if (!propDecl.getters.empty() && !propDecl.setters.empty()) {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
        }

        astToFormatSource.AddBodyMembers(doc, propDecl.setters, level + 1);
        doc.members.emplace_back(DocType::LINE, level, "");
        doc.members.emplace_back(DocType::STRING, level, "}");
    }
}
} // namespace Cangjie::Format