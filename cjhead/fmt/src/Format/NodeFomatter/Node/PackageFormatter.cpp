// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/PackageFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void PackageFormatter::AddPackage(Doc& doc, const AST::Package& package, int level)
{
    doc.type = DocType::FILE;
    doc.indent = level;
    for (auto& it : package.files) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(it.get(), level));
    }
}

void PackageFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto package = As<ASTKind::PACKAGE>(node);
    AddPackage(doc, *package, level);
}
} // namespace Cangjie::Format
