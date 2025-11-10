// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/RefExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void RefExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto refExpr = As<ASTKind::REF_EXPR>(node);
    AddRefExpr(doc, *refExpr, level);
}
void RefExprFormatter::AddRefExpr(Doc& doc, const Cangjie::AST::RefExpr& refExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, refExpr.ref.identifier.GetRawText());
    if (!refExpr.typeArguments.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "<");
        for (auto& n : refExpr.typeArguments) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level + 1));
            if (n != refExpr.typeArguments.back()) {
                doc.members.emplace_back(DocType::STRING, level + 1, ", ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, ">");
    }
    if (refExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format
