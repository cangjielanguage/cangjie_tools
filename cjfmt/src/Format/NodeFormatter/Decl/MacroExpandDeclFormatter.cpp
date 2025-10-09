// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/MacroExpandDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void MacroExpandDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto macroExpandDecl = As<ASTKind::MACRO_EXPAND_DECL>(node);
    astToFormatSource.AddMacroExpandNode(doc, macroExpandDecl, level, funcOptions.patternOrEnum, false, funcOptions);
}
} // namespace Cangjie::Format