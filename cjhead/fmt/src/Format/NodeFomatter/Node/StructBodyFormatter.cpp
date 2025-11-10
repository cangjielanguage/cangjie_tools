// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/StructBodyFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void StructBodyFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto structBody = As<ASTKind::STRUCT_BODY>(node);
    AddStructBody(doc, *structBody, level);
}

void StructBodyFormatter::AddStructBody(Doc& doc, const Cangjie::AST::StructBody& structBody, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (structBody.decls.empty()) {
        doc.members.emplace_back(DocType::STRING, level, " {}");
        return;
    }
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    astToFormatSource.AddBodyMembers(doc, structBody.decls, level + 1);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}
} // namespace Cangjie::Format