// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/GenericParamDeclFormatter.h"
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void GenericParamDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto genericParamDecl = As<ASTKind::GENERIC_PARAM_DECL>(node);
    AddGenericParamDecl(doc, *genericParamDecl, level);
}
void GenericParamDeclFormatter::AddGenericParamDecl(
    Doc& doc, const Cangjie::AST::GenericParamDecl& genericParamDecl, int level)
{
    doc.type = DocType::STRING;
    doc.indent = level;
    doc.value = genericParamDecl.identifier.GetRawText();
}
} // namespace Cangjie::Format