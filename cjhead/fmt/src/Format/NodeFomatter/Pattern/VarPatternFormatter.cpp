// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Pattern/VarPatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void VarPatternFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::VAR_PATTERN>(node);
    AddVarPattern(doc, *pattern, level);
}

void VarPatternFormatter::AddVarPattern(Doc& doc, const VarPattern& varPattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (varPattern.varDecl) {
        FuncOptions funcOptions;
        funcOptions.patternOrEnum = true;
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varPattern.varDecl.get(), level, funcOptions));
    }
}
} // namespace Cangjie::Format