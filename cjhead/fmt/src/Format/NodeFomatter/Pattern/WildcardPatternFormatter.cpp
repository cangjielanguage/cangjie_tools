// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Pattern/WildcardPatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::WildcardPatternFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node>, int level, FuncOptions&)
{
    doc.type = DocType::STRING;
    doc.indent = level;
    doc.value = "_";
}
} // namespace Cangjie::Format
