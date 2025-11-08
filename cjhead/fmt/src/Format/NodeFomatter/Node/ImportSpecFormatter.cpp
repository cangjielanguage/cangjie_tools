// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/ImportSpecFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void ImportSpecFormatter::AddImportSpec(Doc& doc, const Cangjie::AST::ImportSpec& importSpec, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (importSpec.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }

    if (!importSpec.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, importSpec.annotations, level);
    }

    if (importSpec.modifier) {
        astToFormatSource.AddModifier(doc, *importSpec.modifier, level);
    }

    const auto& ic = importSpec.content;
    doc.members.emplace_back(DocType::STRING, level, "import ");
    if (ic.kind != ImportKind::IMPORT_MULTI) {
        AddImportContent(doc, ic, level);
        return;
    }

    if (auto prefix = Utils::JoinStrings(ic.prefixPaths, "."); !prefix.empty()) {
        doc.members.emplace_back(DocType::STRING, level, prefix + ".");
    }
    if (ic.leftCurlPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "{");
    }
    auto it = ic.items.begin();
    if (it != ic.items.end()) {
        doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
        AddImportContent(doc, *it, level);
        ++it;
    }
    for (; it != ic.items.end(); ++it) {
        doc.members.emplace_back(DocType::STRING, level, ", ");
        doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
        AddImportContent(doc, *it, level);
    }
    if (ic.rightCurlPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
        doc.members.emplace_back(DocType::STRING, level, "}");
    }
}

void ImportSpecFormatter::AddImportContent(Doc& doc, const Cangjie::AST::ImportContent& content, int level)
{
    if (content.kind == ImportKind::IMPORT_MULTI) {
        return;
    }
    auto prefix = Utils::JoinStrings(content.prefixPaths, ".");
    doc.members.emplace_back(
        DocType::STRING, level, prefix.empty() ? content.identifier : prefix + "." + content.identifier);
    if (content.kind == ImportKind::IMPORT_ALIAS) {
        doc.members.emplace_back(DocType::STRING, level, " as ");
        if (!content.aliasName.Val().empty()) {
            doc.members.emplace_back(DocType::STRING, level, content.aliasName.Val());
        }
    }
}

void ImportSpecFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto importSpec = As<ASTKind::IMPORT_SPEC>(node);
    AddImportSpec(doc, *importSpec, level);
}
} // namespace Cangjie::Format
