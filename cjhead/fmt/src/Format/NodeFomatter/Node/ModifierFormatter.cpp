// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/ModifierFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::ModifierFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto modifier = As<ASTKind::MODIFIER>(node);
    if (modifier == nullptr) {
        return;
    }
    doc.type = DocType::STRING;
    doc.indent = level;
    doc.value = TOKENS[static_cast<int>(modifier->modifier)];
}
} // namespace Cangjie::Format
