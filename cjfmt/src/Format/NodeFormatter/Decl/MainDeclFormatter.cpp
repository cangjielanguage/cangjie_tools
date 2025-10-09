// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/MainDeclFormatter.h"
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void MainDeclFormatter::AddMainDecl(Doc &doc, const Cangjie::AST::MainDecl &mainDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!mainDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, mainDecl.annotations, level);
    }

    if (mainDecl.TestAttr(Attribute::UNSAFE)) {
        doc.members.emplace_back(DocType::STRING, level, "unsafe ");
    }
    doc.members.emplace_back(DocType::STRING, level, "main");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(mainDecl.funcBody.get(), level));
}

void MainDeclFormatter::ASTToDoc(Doc &doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto mainDecl = As<ASTKind::MAIN_DECL>(node);
    AddMainDecl(doc, *mainDecl, level);
}
}