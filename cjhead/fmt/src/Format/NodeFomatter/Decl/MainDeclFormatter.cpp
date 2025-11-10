// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/MainDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void MainDeclFormatter::AddMainDecl(Doc &doc, const Cangjie::AST::MainDecl &mainDecl, int level)
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

    if (!mainDecl.comments.IsEmpty()) {
        for (auto &lead_comment : mainDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms) {
                addComment(comment.info.Value());
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }
    if (!mainDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, mainDecl.annotations, level);
    }

    if (mainDecl.TestAttr(Attribute::UNSAFE)) {
        doc.members.emplace_back(DocType::STRING, level, "unsafe ");
    }
    doc.members.emplace_back(DocType::STRING, level, "main");
    doc.members.emplace_back(astToFormatSource.ASTToDoc(mainDecl.funcBody.get(), level));
}
void MainDeclFormatter::ASTToDoc(Doc &doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto mainDecl = As<ASTKind::MAIN_DECL>(node);
    AddMainDecl(doc, *mainDecl, level);
}
}