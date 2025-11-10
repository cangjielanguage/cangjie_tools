// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/PropDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void PropDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto propDecl = As<ASTKind::PROP_DECL>(node);
    AddPropDecl(doc, *propDecl, level);
}
void PropDeclFormatter::AddPropDecl(Doc& doc, const Cangjie::AST::PropDecl& propDecl, int level)
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

    if (!propDecl.comments.IsEmpty()) {
        for (auto &lead_comment : propDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms) {
                addComment(comment.info.Value());
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }

    if (!propDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, propDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, propDecl.modifiers, level);

    doc.members.emplace_back(DocType::STRING, level, "prop ");
    doc.members.emplace_back(DocType::STRING, level, propDecl.identifier.GetRawText());
    if (propDecl.type) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(propDecl.type.get(), level));
    }
    if (propDecl.initializer) {
        doc.members.emplace_back(DocType::STRING, level, " = ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(propDecl.initializer.get(), level));
    }
}
} // namespace Cangjie::Format