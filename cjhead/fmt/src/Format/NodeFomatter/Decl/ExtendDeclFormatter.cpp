// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/ExtendDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include <algorithm>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

bool Match(const std::string& str)
{
    if (str.find("Emitted by MacroCall") != std::string::npos) {
        return true;
    }
    std::regex pattern(R"(^/\* \d+\.\d+ \*/$)");
    return std::regex_match(str, pattern);
};

void ExtendDeclFormatter::AddExtendDecl(Doc& doc, const Cangjie::AST::ExtendDecl& extendDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    std::unordered_set<std::string> comment_set;

    auto addComment =
        [*this, &comment_set, &doc, &level](std::string comment) {
            if (!Match(comment) && !comment_set.count(comment)) {
                comment_set.insert(comment);
                doc.members.emplace_back(DocType::STRING, level, comment);
            }
        };
    if (!extendDecl.comments.IsEmpty()) {
        for (auto &lead_comment : extendDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms)
                addComment(comment.info.Value());
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }
    if (!extendDecl.annotations.empty())
        astToFormatSource.AddAnnotations(doc, extendDecl.annotations, level);
    doc.members.emplace_back(DocType::STRING, level, "extend");
    if (extendDecl.generic)
        astToFormatSource.AddGenericParams(doc, *extendDecl.generic, level);
    doc.members.emplace_back(DocType::STRING, level, " ");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(extendDecl.extendedType.get(), level));
    if (!extendDecl.inheritedTypes.empty())
        AddExtendDeclInheritedTypes(doc, extendDecl, level);
    if (extendDecl.generic) {
        extendDecl.generic->genericConstraints.erase(
            std::remove_if(extendDecl.generic->genericConstraints.begin(), extendDecl.generic->genericConstraints.end(),
                [](auto& x) {
                    auto& cst_type = x->type;
                    return cst_type->ToString() == "";
                }),
            extendDecl.generic->genericConstraints.end());
        astToFormatSource.AddGenericBound(doc, *extendDecl.generic, level);
    }
    if (extendDecl.members.empty()) {
        doc.members.emplace_back(DocType::STRING, level, " {}");
        return;
    }
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    astToFormatSource.AddBodyMembers(doc, extendDecl.members, level + 1);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void ExtendDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto extendDecl = As<ASTKind::EXTEND_DECL>(node);
    AddExtendDecl(doc, *extendDecl, level);
}
void ExtendDeclFormatter::AddExtendDeclInheritedTypes(Doc& doc, const Cangjie::AST::ExtendDecl& extendDecl, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& trait : extendDecl.inheritedTypes) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(trait.get(), level + 1));
        if (trait != extendDecl.inheritedTypes.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            group.members.emplace_back(DocType::STRING, level + 1, "&");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}
} // namespace Cangjie::Format
