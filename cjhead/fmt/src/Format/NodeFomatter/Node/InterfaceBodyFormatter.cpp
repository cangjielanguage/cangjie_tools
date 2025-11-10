// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/InterfaceBodyFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void InterfaceBodyFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto interfaceBody = As<ASTKind::INTERFACE_BODY>(node);
    AddInterfaceBody(doc, *interfaceBody, level);
}
void InterfaceBodyFormatter::AddInterfaceBody(Doc& doc, const Cangjie::AST::InterfaceBody& interfaceBody, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (interfaceBody.decls.empty()) {
        doc.members.emplace_back(DocType::STRING, level, " {}");
        return;
    }
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    astToFormatSource.AddBodyMembers(doc, interfaceBody.decls, level + 1);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}
} // namespace Cangjie::Format