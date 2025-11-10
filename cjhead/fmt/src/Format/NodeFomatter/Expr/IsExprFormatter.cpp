// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/IsExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void IsExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto isExpr = As<ASTKind::IS_EXPR>(node);
    AddIsExpr(doc, *isExpr, level);
}
void IsExprFormatter::AddIsExpr(Doc& doc, const Cangjie::AST::IsExpr& isExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    Doc group(DocType::GROUP, level, "");
    if (isExpr.leftExpr) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(isExpr.leftExpr.get(), level));
    }
    group.members.emplace_back(DocType::STRING, level, " is ");
    if (isExpr.isType) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(isExpr.isType.get(), level));
    }
    doc.members.emplace_back(group);
}
} // namespace Cangjie::Format