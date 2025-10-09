// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/MacroDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void MacroDeclFormatter::AddMacroDecl(Doc& doc, const Cangjie::AST::MacroDecl& macroDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!macroDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, macroDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, macroDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "macro " + macroDecl.identifier.GetRawText());
    doc.members.emplace_back(astToFormatSource.ASTToDoc(macroDecl.funcBody.get(), level));
}

void MacroDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto macroDecl = As<ASTKind::MACRO_DECL>(node);
    AddMacroDecl(doc, *macroDecl, level);
}
} // namespace Cangjie::Format
