// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/OptionalExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void OptionalExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto optionalExpr = As<ASTKind::OPTIONAL_EXPR>(node);
    AddOptionalExpr(doc, *optionalExpr, level);
}
void OptionalExprFormatter::AddOptionalExpr(Doc& doc, const Cangjie::AST::OptionalExpr& optionalExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(astToFormatSource.ASTToDoc(optionalExpr.baseExpr.get(), level));
    if (optionalExpr.questPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "?");
    }
}
} // namespace Cangjie::Format