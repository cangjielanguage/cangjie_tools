// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/FuncDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void FuncDeclFormatter::AddFuncDecl(
    Doc& doc, const Cangjie::AST::FuncDecl& funcDecl, int level, FuncOptions funcOptions)
{
    if (funcDecl.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!funcDecl.annotations.empty()) {
        if (funcOptions.patternOrEnum) {
            astToFormatSource.AddAnnotations(doc, funcDecl.annotations, level, false);
        } else {
            astToFormatSource.AddAnnotations(doc, funcDecl.annotations, level);
        }
    }

    astToFormatSource.AddModifier(doc, funcDecl.modifiers, level);
    if (!funcDecl.TestAttr(Attribute::CONSTRUCTOR) && !funcOptions.patternOrEnum &&
        (!funcDecl.isGetter && !funcDecl.isSetter)) {
        if (funcDecl.TestAttr(Attribute::MACRO_FUNC)) {
            doc.members.emplace_back(DocType::STRING, level, "macro ");
        } else if (!funcDecl.IsFinalizer()) {
            doc.members.emplace_back(DocType::STRING, level, "func ");
        }
        doc.members.emplace_back(DocType::STRING, level, funcDecl.identifier.GetRawText());
    } else {
        doc.members.emplace_back(DocType::STRING, level, funcDecl.identifier.GetRawText());
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(funcDecl.funcBody.get(), level, funcOptions));
}

void FuncDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto funcDecl = As<ASTKind::FUNC_DECL>(node);
    AddFuncDecl(doc, *funcDecl, level, funcOptions);
}
} // namespace Cangjie::Format
