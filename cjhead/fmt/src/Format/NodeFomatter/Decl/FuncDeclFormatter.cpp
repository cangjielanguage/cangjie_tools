// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/FuncDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void FuncDeclFormatter::AddFuncDecl(
    Doc& doc, const Cangjie::AST::FuncDecl& funcDecl, int level, FuncOptions funcOptions)
{
    if (funcDecl.TestAttr(Attribute::COMPILER_ADD)) {
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

    if (!funcDecl.comments.IsEmpty()) {
        for (auto &lead_comment : funcDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms) {
                addComment(comment.info.Value());
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!funcDecl.annotations.empty()) {
        if (funcOptions.patternOrEnum) {
            astToFormatSource.AddAnnotations(doc, funcDecl.annotations, level, false);
        } else {
            astToFormatSource.AddAnnotations(doc, funcDecl.annotations, level);
        }
    }

    astToFormatSource.AddModifier(doc, funcDecl.modifiers, level);
    if (!funcDecl.TestAttr(Attribute::CONSTRUCTOR) && !funcOptions.patternOrEnum &&
        (!funcDecl.isGetter && !funcDecl.isSetter)) {
        if (funcDecl.TestAttr(Attribute::MACRO_FUNC)) {
            doc.members.emplace_back(DocType::STRING, level, "macro ");
        } else if (!funcDecl.IsFinalizer()) {
            doc.members.emplace_back(DocType::STRING, level, "func ");
        }
        doc.members.emplace_back(DocType::STRING, level, funcDecl.identifier.GetRawText());
    } else {
        doc.members.emplace_back(DocType::STRING, level, funcDecl.identifier.GetRawText());
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(funcDecl.funcBody.get(), level, funcOptions));
}
void FuncDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto funcDecl = As<ASTKind::FUNC_DECL>(node);
    AddFuncDecl(doc, *funcDecl, level, funcOptions);
}
} // namespace Cangjie::Format
