// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Decl/EnumDeclFormatter.h"
#include <regex>
#include <string>
#include <unordered_set>
#include "cangjie/AST/Node.h"
#include "Format/ASTToFormatSource.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

bool IsMatch(const std::string& str)
{
    if (str.find("Emitted by MacroCall") != std::string::npos) {
        return true;
    }
    std::regex pattern(R"(^/\* \d+\.\d+ \*/$)");
    return std::regex_match(str, pattern);
};

void AddComment(std::unordered_set<std::string>& comment_set, Doc& doc, int level, std::string comment)
{
    if (!IsMatch(comment) && !comment_set.count(comment)) {
        comment_set.insert(comment);
        doc.members.emplace_back(DocType::STRING, level, comment);
    }
}

void EnumDeclFormatter::AddEnumDecl(Doc& doc, const Cangjie::AST::EnumDecl& enumDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    std::unordered_set<std::string> comment_set;
    if (!enumDecl.comments.IsEmpty()) {
        for (auto &lead_comment : enumDecl.comments.leadingComments) {
            for (auto &comment : lead_comment.cms)
                AddComment(comment_set, doc, level, comment.info.Value());
            doc.members.emplace_back(DocType::LINE, level, "");
        }
    }

    if (!enumDecl.annotations.empty())
        astToFormatSource.AddAnnotations(doc, enumDecl.annotations, level);
    astToFormatSource.AddModifier(doc, enumDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "enum ");
    doc.members.emplace_back(DocType::STRING, level, enumDecl.identifier.GetRawText());
    auto& generic = enumDecl.generic;
    if (generic)
        astToFormatSource.AddGenericParams(doc, *generic, level);
    if (!enumDecl.inheritedTypes.empty())
        AddEnumInheritedTypes(doc, enumDecl, level);
    if (generic)
        astToFormatSource.AddGenericBound(doc, *generic, level);
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    if (enumDecl.constructors.size() > 1 &&
        enumDecl.constructors.front()->begin.line != enumDecl.constructors.back()->begin.line) {
        AddEnumBreakLineConstructors(doc, enumDecl, level);
        return;
    }
    Doc valueGroup(DocType::GROUP, level + 1, "");
    FuncOptions funcOptions;
    funcOptions.patternOrEnum = true;
    for (auto& cons : enumDecl.constructors) {
        if (cons != enumDecl.constructors.front() || HasFirstOptionalSeparator(enumDecl))
            valueGroup.members.emplace_back(DocType::STRING, level + 1, "| ");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        valueGroup.members.emplace_back(astToFormatSource.ASTToDoc(cons.get(), level + 1, funcOptions));
        if (cons != enumDecl.constructors.back())
            valueGroup.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
    }
    if (enumDecl.hasEllipsis) {
        valueGroup.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        valueGroup.members.emplace_back(DocType::STRING, level + 1, "| ...");
    }
    doc.members.emplace_back(valueGroup);
    AddEnumMembers(doc, enumDecl, level);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void EnumDeclFormatter::AddEnumMembers(Doc& doc, const Cangjie::AST::EnumDecl& enumDecl, int level)
{
    if (!enumDecl.members.empty()) {
        doc.members.emplace_back(DocType::SEPARATE, level + 1, "");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        astToFormatSource.AddBodyMembers(doc, enumDecl.members, level + 1);
    }
}

void EnumDeclFormatter::AddEnumInheritedTypes(Doc& doc, const Cangjie::AST::EnumDecl& enumDecl, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& type : enumDecl.inheritedTypes) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(type.get(), level + 1));
        if (type != enumDecl.inheritedTypes.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            group.members.emplace_back(DocType::STRING, level + 1, "&");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}

void EnumDeclFormatter::AddEnumBreakLineConstructors(Doc& doc, const Cangjie::AST::EnumDecl& enumDecl, int level)
{
    auto& constructors = enumDecl.constructors;
    bool firstConstructorHasBitOr = false;
    if (!constructors.empty() && !enumDecl.bitOrPosVector.empty() &&
        (constructors.front()->begin > enumDecl.bitOrPosVector.front())) {
        firstConstructorHasBitOr = true;
    }
    bool first = true;
    FuncOptions funcOptions;
    funcOptions.patternOrEnum = true;
    for (auto& cons : constructors) {
        if (!first || firstConstructorHasBitOr) {
            doc.members.emplace_back(DocType::STRING, level + 1, "| ");
        }
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        first = false;
        doc.members.emplace_back(astToFormatSource.ASTToDoc(cons.get(), level + 1, funcOptions));
        if (cons != constructors.back()) {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
        }
    }
    if (enumDecl.hasEllipsis) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        doc.members.emplace_back(DocType::STRING, level + 1, "| ...");
    }
    if (!enumDecl.members.empty()) {
        doc.members.emplace_back(DocType::SEPARATE, level + 1, "");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        astToFormatSource.AddBodyMembers(doc, enumDecl.members, level + 1);
    }
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void EnumDeclFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto enumDecl = As<ASTKind::ENUM_DECL>(node);
    AddEnumDecl(doc, *enumDecl, level);
}
bool EnumDeclFormatter::HasFirstOptionalSeparator(const EnumDecl& ed)
{
    if (!ed.constructors.empty() && !ed.bitOrPosVector.empty() &&
        (ed.constructors.front()->begin > ed.bitOrPosVector.front())) {
        return true;
    }
    if (ed.bitOrPosVector.empty()) { // enum E { }
        return false;
    }
    if (ed.constructors.empty()) { // enum E { | }
        return true;
    }
    return ed.bitOrPosVector.front() < ed.constructors.front()->GetBegin();
}
} // namespace Cangjie::Format