// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/VarWithPatternDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void VarWithPatternDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto varWithPatternDecl = As<ASTKind::VAR_WITH_PATTERN_DECL>(node);
    AddVarWithPatternDecl(doc, *varWithPatternDecl, level);
}
void VarWithPatternDeclFormatter::AddVarWithPatternDecl(
    Doc& doc, const Cangjie::AST::VarWithPatternDecl& varWithPatternDecl, int level)
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

    if (!varWithPatternDecl.comments.IsEmpty()) {
        for (auto &lead_comment : varWithPatternDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms) {
                addComment(comment.info.Value());
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }

    if (!varWithPatternDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, varWithPatternDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, varWithPatternDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level,
        varWithPatternDecl.isVar ? "var " : varWithPatternDecl.isConst ? "" : "let ");

    if (varWithPatternDecl.irrefutablePattern) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.irrefutablePattern.get(), level));
    }

    if (varWithPatternDecl.type) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.type.get(), level));
    }

    if (varWithPatternDecl.initializer) {
        doc.members.emplace_back(DocType::STRING, level, " = ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.initializer.get(), level));
    }
}
} // namespace Cangjie::Format