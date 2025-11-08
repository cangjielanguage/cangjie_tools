// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/VarDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void VarDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto varDecl = As<ASTKind::VAR_DECL>(node);
    AddVarDecl(doc, *varDecl, level, funcOptions);
}
void VarDeclFormatter::AddVarDecl(Doc& doc, const Cangjie::AST::VarDecl& varDecl, int level, FuncOptions funcOptions)
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

    if (!varDecl.comments.IsEmpty()) {
        for (auto &lead_comment : varDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms) {
                addComment(comment.info.Value());
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }

    if (!varDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, varDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, varDecl.modifiers, level);
    if (!funcOptions.patternOrEnum) {
        doc.members.emplace_back(DocType::STRING, level, varDecl.isVar ? "var " : varDecl.isConst ? "" : "let ");
    }

    doc.members.emplace_back(DocType::STRING, level, varDecl.identifier.GetRawText());
    if (varDecl.type) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varDecl.type.get(), level));
    }
    if (varDecl.initializer) {
        doc.members.emplace_back(DocType::STRING, level, " = ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varDecl.initializer.get(), level));
    }
}
} // namespace Cangjie::Format