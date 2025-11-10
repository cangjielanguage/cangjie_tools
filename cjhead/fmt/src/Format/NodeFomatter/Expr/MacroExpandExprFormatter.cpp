// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/MacroExpandExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
#include <string>

namespace Cangjie::Format {
using namespace Cangjie::AST;

void MacroExpandExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto macroExpandExpr = As<ASTKind::MACRO_EXPAND_EXPR>(node);
    AddMacroExpandExpr(doc, *macroExpandExpr, level);
}
void MacroExpandExprFormatter::AddMacroExpandExpr(
    Doc& doc, const Cangjie::AST::MacroExpandExpr& macroExpandExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    std::string compileTimeVisibleStr = macroExpandExpr.invocation.isCompileTimeVisible ? "!" : "";
    std::string macroStr = "@" + compileTimeVisibleStr + macroExpandExpr.invocation.fullName;

    if (macroExpandExpr.invocation.leftSquarePos != INVALID_POSITION &&
        macroExpandExpr.invocation.rightSquarePos != INVALID_POSITION) {
        macroStr += astToFormatSource.sm.GetContentBetween(macroExpandExpr.invocation.leftSquarePos.fileID,
            macroExpandExpr.invocation.leftSquarePos, macroExpandExpr.invocation.rightSquarePos + 1);
    }

    if (macroExpandExpr.invocation.leftParenPos != INVALID_POSITION &&
        macroExpandExpr.invocation.rightParenPos != INVALID_POSITION) {
        macroStr += astToFormatSource.sm.GetContentBetween(macroExpandExpr.invocation.leftParenPos.fileID,
            macroExpandExpr.invocation.leftParenPos, macroExpandExpr.invocation.rightParenPos + 1);
    }

    doc.members.emplace_back(DocType::STRING, level, macroStr);
    if (macroExpandExpr.invocation.decl != nullptr) {
        if (macroExpandExpr.invocation.decl->begin.line == macroExpandExpr.invocation.identifierPos.line) {
            doc.members.emplace_back(DocType::STRING, level, " ");
        } else {
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(macroExpandExpr.invocation.decl.get(), level));
    }
}
} // namespace Cangjie::Format