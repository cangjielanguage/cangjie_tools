// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/PrimaryCtorDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void PrimaryCtorDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto primaryCtorDecl = As<ASTKind::PRIMARY_CTOR_DECL>(node);
    AddPrimaryCtorDecl(doc, *primaryCtorDecl, level, funcOptions);
}
void PrimaryCtorDeclFormatter::AddPrimaryCtorDecl(
    Doc& doc, const Cangjie::AST::PrimaryCtorDecl& primaryCtorDecl, int level, FuncOptions funcOptions)
{
    if (primaryCtorDecl.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }

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

    if (!primaryCtorDecl.comments.IsEmpty()) {
        for (auto &lead_comment : primaryCtorDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms) {
                addComment(comment.info.Value());
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!primaryCtorDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, primaryCtorDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, primaryCtorDecl.modifiers, level);
    if (!primaryCtorDecl.TestAttr(Attribute::CONSTRUCTOR) && !funcOptions.patternOrEnum) {
        doc.members.emplace_back(DocType::STRING, level, "func ");
        doc.members.emplace_back(DocType::STRING, level, primaryCtorDecl.identifier.GetRawText());
    } else {
        doc.members.emplace_back(DocType::STRING, level, primaryCtorDecl.identifier.GetRawText());
    }
    doc.members.emplace_back(
        astToFormatSource.ASTToDoc(primaryCtorDecl.funcBody.get(), level, funcOptions));
}
} // namespace Cangjie::Format
