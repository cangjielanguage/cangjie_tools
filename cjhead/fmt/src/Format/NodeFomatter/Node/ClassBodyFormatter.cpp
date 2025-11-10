// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/ClassBodyFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ClassBodyFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto classBody = As<ASTKind::CLASS_BODY>(node);
    AddClassBody(doc, *classBody, level);
}
void ClassBodyFormatter::AddClassBody(Doc& doc, const Cangjie::AST::ClassBody& classBody, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (classBody.decls.empty()) {
        // class test {}
        astToFormatSource.AddEmptyBody(doc, classBody, level);
    } else {
        // class test {body}
        doc.members.emplace_back(DocType::STRING, level, " {");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        astToFormatSource.AddBodyMembers(doc, classBody.decls, level + 1);
        doc.members.emplace_back(DocType::LINE, level, "");
        doc.members.emplace_back(DocType::STRING, level, "}");
    }
}
} // namespace Cangjie::Format