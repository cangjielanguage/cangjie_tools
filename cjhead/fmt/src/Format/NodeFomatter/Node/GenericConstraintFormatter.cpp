// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/GenericConstraintFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void GenericConstraintFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto genericConstraint = As<ASTKind::GENERIC_CONSTRAINT>(node);
    AddGenericConstraint(doc, *genericConstraint, level);
}
void GenericConstraintFormatter::AddGenericConstraint(
    Doc& doc, const Cangjie::AST::GenericConstraint& genericConstraint, int level)
{
    doc.type = DocType::GROUP;
    doc.indent = level;

    if (genericConstraint.wherePos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, " where ");
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(genericConstraint.type.get()));
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    for (auto& ub : genericConstraint.upperBounds) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(ub.get()));
        if (ub != genericConstraint.upperBounds.back()) {
            doc.members.emplace_back(DocType::STRING, level, " & ");
        }
    }
}
} // namespace Cangjie::Format