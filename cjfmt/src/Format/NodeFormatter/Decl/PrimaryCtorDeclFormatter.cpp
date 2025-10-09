// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/PrimaryCtorDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void PrimaryCtorDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto primaryCtorDecl = As<ASTKind::PRIMARY_CTOR_DECL>(node);
    AddPrimaryCtorDecl(doc, *primaryCtorDecl, level, funcOptions);
}

void PrimaryCtorDeclFormatter::AddPrimaryCtorDecl(
    Doc& doc, const Cangjie::AST::PrimaryCtorDecl& primaryCtorDecl, int level, FuncOptions funcOptions)
{
    if (primaryCtorDecl.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!primaryCtorDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, primaryCtorDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, primaryCtorDecl.modifiers, level);
    if (!primaryCtorDecl.TestAttr(Attribute::CONSTRUCTOR) && !funcOptions.patternOrEnum) {
        doc.members.emplace_back(DocType::STRING, level, "func ");
        doc.members.emplace_back(DocType::STRING, level, primaryCtorDecl.identifier.GetRawText());
    } else {
        doc.members.emplace_back(DocType::STRING, level, primaryCtorDecl.identifier.GetRawText());
    }
    doc.members.emplace_back(
        astToFormatSource.ASTToDoc(primaryCtorDecl.funcBody.get(), level, funcOptions));
}
} // namespace Cangjie::Format
