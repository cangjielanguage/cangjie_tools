// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/OptionalChainExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::OptionalChainExprFormatter::ASTToDoc(
    Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto optionalChainExpr = As<ASTKind::OPTIONAL_CHAIN_EXPR>(node);
    AddOptionalChainExpr(doc, *optionalChainExpr, level);
}
void OptionalChainExprFormatter::AddOptionalChainExpr(
    Doc& doc, const Cangjie::AST::OptionalChainExpr& optionalChainExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(astToFormatSource.ASTToDoc(optionalChainExpr.expr.get(), level));
}
} // namespace Cangjie::Format
