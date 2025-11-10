// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/ConstantTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
namespace Cangjie::Format {
using namespace Cangjie::AST;
void ConstantTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::CONSTANT_TYPE>(node);
    AddConstantType(doc, *type, level);
}

void ConstantTypeFormatter::AddConstantType(Doc& doc, const Cangjie::AST::ConstantType& constantType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "$");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(constantType.constantExpr.get(), level));
}
} // namespace Cangjie::Format