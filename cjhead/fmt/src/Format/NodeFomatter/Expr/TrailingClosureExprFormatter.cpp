// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/TrailingClosureExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void TrailingClosureExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto trailingClosureExpr = StaticAs<ASTKind::TRAIL_CLOSURE_EXPR>(node);
    AddTrailingClosureExpr(doc, *trailingClosureExpr, level);
}
void TrailingClosureExprFormatter::AddTrailingClosureExpr(
    Doc& doc, const Cangjie::AST::TrailingClosureExpr& trailingClosureExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    auto& node = trailingClosureExpr.expr;
    Doc group(DocType::GROUP, level, "");
    if (node) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(node.get(), level));
    }
    group.members.emplace_back(DocType::STRING, level, " ");
    FuncOptions funcOptions;
    funcOptions.isLambda = true;
    if (trailingClosureExpr.lambda) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(trailingClosureExpr.lambda.get(), level, funcOptions));
    }
    doc.members.emplace_back(group);
}
} // namespace Cangjie::Format
