// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/InterfaceDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void InterfaceDeclFormatter::AddInterfaceDecl(Doc& doc, const Cangjie::AST::InterfaceDecl& interfaceDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    std::unordered_set<std::string> comment_set;

    auto isMatch = [](const std::string& str) {
        if (str.find("Emitted by MacroCall") != std::string::npos) {
            return true;
        }

        std::regex pattern(R"(^/\* \d+\.\d+ \*/$)");
        return std::regex_match(str, pattern);
    };

    auto addComment =
        [*this, &comment_set, &isMatch, &doc, &level](std::string comment) {
            if (!isMatch(comment) && !comment_set.count(comment)) {
                comment_set.insert(comment);
                doc.members.emplace_back(DocType::STRING, level, comment);
            }
        };

    if (!interfaceDecl.comments.IsEmpty()) {
        for (auto &lead_comment : interfaceDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms) {
                addComment(comment.info.Value());
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }

    if (!interfaceDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, interfaceDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, interfaceDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "interface ");
    doc.members.emplace_back(DocType::STRING, level, interfaceDecl.identifier.GetRawText());
    auto& generic = interfaceDecl.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    if (!interfaceDecl.inheritedTypes.empty()) {
        AddInterfaceInheritedTypes(doc, interfaceDecl, level);
    }
    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(interfaceDecl.body.get(), level));
}

void InterfaceDeclFormatter::AddInterfaceInheritedTypes(
    Doc& doc, const Cangjie::AST::InterfaceDecl& interfaceDecl, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& type : interfaceDecl.inheritedTypes) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(type.get(), level + 1));
        if (type != interfaceDecl.inheritedTypes.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            group.members.emplace_back(DocType::STRING, level + 1, "&");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}

void InterfaceDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto interfaceDecl = As<ASTKind::INTERFACE_DECL>(node);
    AddInterfaceDecl(doc, *interfaceDecl, level);
}
} // namespace Cangjie::Format