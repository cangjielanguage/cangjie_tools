// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Node/PackageSpecFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void PackageSpecFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto spec = As<ASTKind::PACKAGE_SPEC>(node);
    AddPackageSpec(doc, *spec, level);
}

void PackageSpecFormatter::AddPackageSpec(Doc& doc, const Cangjie::AST::PackageSpec& packageSpec, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (packageSpec.modifier) {
        astToFormatSource.AddModifier(doc, *packageSpec.modifier, level);
    }
    if (packageSpec.hasMacro) {
        doc.members.emplace_back(DocType::STRING, level, "macro ");
    }

    auto prefix = Utils::JoinStrings(packageSpec.prefixPaths, ".");
    auto prefixResult = prefix + ".";
    if (packageSpec.hasDoubleColon) {
        size_t firstDotPos = prefixResult.find('.');
        if (firstDotPos != std::string::npos) {
            prefixResult.replace(firstDotPos, 1, "::");
        }
    }
    doc.members.emplace_back(DocType::STRING, level,
        prefix.empty() ? "package " + packageSpec.packageName : "package " + prefixResult + packageSpec.packageName);
}
} // namespace Cangjie::Format
