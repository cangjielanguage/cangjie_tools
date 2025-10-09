// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Decl/MacroExpandParamFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
namespace Cangjie::Format {
using namespace Cangjie::AST;

void MacroExpandParamFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto macroExpandParam = As<ASTKind::MACRO_EXPAND_PARAM>(node);
    astToFormatSource.AddMacroExpandNode(doc, macroExpandParam, level, funcOptions.patternOrEnum, true, funcOptions);
}
} // namespace Cangjie::Format