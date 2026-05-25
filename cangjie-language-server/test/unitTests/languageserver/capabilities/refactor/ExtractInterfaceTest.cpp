// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include "cangjie/AST/Node.h"
#include "CompletionType.h"
#include <nlohmann/json.hpp>
#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

using namespace Cangjie::AST;

namespace ark {
std::string GetVisibility(const Cangjie::AST::Decl &decl);
std::string EscapeJsonString(const std::string &input);
void AppendCommentGroupText(std::string &result, const std::vector<CommentGroup> &groupList);
std::optional<Location> ParseSelectedTypeReferenceEntry(const nlohmann::json &entry);
bool IsSameSelectedTypeReference(const Location &lhs, const Location &rhs);
std::optional<std::pair<Cangjie::Position, Cangjie::Position>> FindTransferableLeadingCommentsRange(
    const Cangjie::AST::Decl &decl);
std::optional<TextEdit> BuildImportInterfaceEditForFile(const File &sourceFile, const std::string &targetPath,
    const std::string &interfaceName);
std::optional<Range> GetNameReferenceRange(const NameReferenceExpr &refExpr);
bool ParseBoolOption(const std::map<std::string, std::string> &options, const std::string &key, bool defaultValue);
std::string ResolveImplementationClassName(const std::map<std::string, std::string> &options,
    const std::string &fallback);
std::string JoinTypes(const std::vector<std::string> &types);
void AddSelectedMemberSignature(std::unordered_set<std::string> &chosen, const std::string &signature);
bool ParseSelectedMembersFromJson(const std::string &raw, std::unordered_set<std::string> &chosen);
void ParseSelectedMembersFromText(const std::string &raw, std::unordered_set<std::string> &chosen);
std::unordered_set<std::string> ParseSelectedMembers(const std::map<std::string, std::string> &options);
std::optional<std::string> ParseInheritedMemberSignature(const std::string &signature);
std::string NormalizeTypeNameForCompare(std::string name);
std::string ToSnakeCaseFileName(const std::string &name);
std::string ResolveTargetFilePath(const std::string &targetPath, const std::string &interfaceName);
bool IsValidPackageName(const std::string &packageName);
} // namespace ark

using namespace ark;

namespace {
CommentGroup MakeCommentGroup(const std::vector<std::tuple<std::string, Cangjie::Position, Cangjie::Position>> &items)
{
    CommentGroup group;
    for (const auto &[text, begin, end] : items) {
        group.cms.push_back(Comment{CommentStyle::LEAD_LINE, CommentKind::LINE,
            Cangjie::Token(Cangjie::TokenKind::COMMENT, text, begin, end)});
    }
    return group;
}
} // namespace

TEST(ExtractInterfaceTest, VisibilityOptionsAndNames)
{
    Decl decl;
    EXPECT_EQ(GetVisibility(decl), "public");
    decl.EnableAttr(Attribute::PUBLIC);
    EXPECT_EQ(GetVisibility(decl), "public");
    decl.DisableAttr(Attribute::PUBLIC);
    decl.EnableAttr(Attribute::PROTECTED);
    EXPECT_EQ(GetVisibility(decl), "protected");
    decl.DisableAttr(Attribute::PROTECTED);
    decl.EnableAttr(Attribute::INTERNAL);
    EXPECT_EQ(GetVisibility(decl), "internal");
    decl.DisableAttr(Attribute::INTERNAL);
    decl.EnableAttr(Attribute::PRIVATE);
    EXPECT_EQ(GetVisibility(decl), "private");

    EXPECT_EQ(EscapeJsonString("a\"b\\c"), "a\\\"b\\\\c");

    std::map<std::string, std::string> options{{"enabled", "true"}, {"disabled", "false"},
        {"implementationClassName", "  PersonImpl  "}};
    EXPECT_TRUE(ParseBoolOption(options, "enabled", false));
    EXPECT_FALSE(ParseBoolOption(options, "disabled", true));
    EXPECT_TRUE(ParseBoolOption(options, "missing", true));
    EXPECT_EQ(ResolveImplementationClassName(options, "Fallback"), "PersonImpl");
    EXPECT_EQ(ResolveImplementationClassName({}, "  Fallback  "), "Fallback");

    EXPECT_EQ(ToSnakeCaseFileName("PersonImpl"), "person_impl");
    EXPECT_EQ(ToSnakeCaseFileName("URLParser2"), "url_parser2");
    EXPECT_EQ(ToSnakeCaseFileName("person impl"), "person_impl");
    EXPECT_EQ(ResolveTargetFilePath("", "Person"), "");
    EXPECT_EQ(ResolveTargetFilePath("D:/tmp/person.cj", "Person"), "D:/tmp/person.cj");
    EXPECT_TRUE(ResolveTargetFilePath("D:/tmp", "PersonImpl").find("person_impl.cj") !=
        std::string::npos);

    EXPECT_TRUE(IsValidPackageName("ohos.demo"));
    EXPECT_FALSE(IsValidPackageName(""));
    EXPECT_FALSE(IsValidPackageName("default"));
    EXPECT_FALSE(IsValidPackageName("a/b"));
    EXPECT_FALSE(IsValidPackageName("a\\b"));
    EXPECT_FALSE(IsValidPackageName("a:b"));
}

TEST(ExtractInterfaceTest, SelectedMembersAndTypes)
{
    EXPECT_EQ(JoinTypes({}), "");
    EXPECT_EQ(JoinTypes({"Base"}), "Base");
    EXPECT_EQ(JoinTypes({"Base", "Serializable", "Debug"}), "Base & Serializable & Debug");

    std::unordered_set<std::string> chosen;
    AddSelectedMemberSignature(chosen, "  func area(radius: Float64): Float64  ");
    EXPECT_TRUE(chosen.count("func area(radius: Float64): Float64") > 0);

    std::unordered_set<std::string> jsonChosen;
    EXPECT_FALSE(ParseSelectedMembersFromJson("", jsonChosen));
    EXPECT_FALSE(ParseSelectedMembersFromJson("{\"bad\":true}", jsonChosen));
    EXPECT_TRUE(ParseSelectedMembersFromJson("[\"func f(): Int64\", 1, \"func g(): Unit\"]",
        jsonChosen));
    EXPECT_TRUE(jsonChosen.count("func f(): Int64") > 0);
    EXPECT_TRUE(jsonChosen.count("func g(): Unit") > 0);

    std::unordered_set<std::string> textChosen;
    ParseSelectedMembersFromText("func a(): Int64, func b(): String", textChosen);
    EXPECT_TRUE(textChosen.count("func a(): Int64") > 0);
    EXPECT_TRUE(textChosen.count("func b(): String") > 0);

    auto parsedFromJson = ParseSelectedMembers({{"selectedMembers", "[\"func c(): Bool\"]"}});
    EXPECT_TRUE(parsedFromJson.count("func c(): Bool") > 0);
    auto parsedFromText = ParseSelectedMembers({{"selectedMembers", "func d(): Unit\nfunc e(): Rune"}});
    EXPECT_TRUE(parsedFromText.count("func d(): Unit") > 0);
    EXPECT_TRUE(parsedFromText.count("func e(): Rune") > 0);
    EXPECT_TRUE(ParseSelectedMembers({}).empty());

    EXPECT_FALSE(ParseInheritedMemberSignature("func f(): Unit").has_value());
    auto inherited = ParseInheritedMemberSignature("<: pkg.Base<Int64>");
    ASSERT_TRUE(inherited.has_value());
    EXPECT_EQ(*inherited, "pkg.Base<Int64>");
    EXPECT_EQ(NormalizeTypeNameForCompare(" pkg.Base<Int64> "), "Base");
    EXPECT_EQ(NormalizeTypeNameForCompare("Simple"), "Simple");
    EXPECT_EQ(NormalizeTypeNameForCompare(""), "");
}

TEST(ExtractInterfaceTest, CommentsSkipDocCommentsAndKeepTransferableRange)
{
    std::vector<CommentGroup> groups = {
        MakeCommentGroup({
            {"", {1, 1, 1}, {1, 1, 1}},
            {"  /** doc comment */", {1, 2, 1}, {1, 2, 20}},
            {"// line comment", {1, 3, 1}, {1, 3, 16}},
            {"/* block comment */", {1, 4, 1}, {1, 4, 20}},
        })
    };

    std::string text = "prefix";
    AppendCommentGroupText(text, groups);
    EXPECT_EQ(text, "prefix\n// line comment\n/* block comment */\n");

    Decl decl;
    EXPECT_FALSE(FindTransferableLeadingCommentsRange(decl).has_value());
    decl.comments.leadingComments = groups;
    auto range = FindTransferableLeadingCommentsRange(decl);
    ASSERT_TRUE(range.has_value());
    EXPECT_EQ(range->first.line, 3);
    EXPECT_EQ(range->first.column, 1);
    EXPECT_EQ(range->second.line, 4);
    EXPECT_EQ(range->second.column, 20);

    Decl docOnly;
    docOnly.comments.leadingComments = {MakeCommentGroup({{"\t/** doc only */", {1, 6, 1}, {1, 6, 17}}})};
    EXPECT_FALSE(FindTransferableLeadingCommentsRange(docOnly).has_value());
}

TEST(ExtractInterfaceTest, ParseSelectedTypeReferenceEntryAndCompareLocation)
{
    EXPECT_FALSE(ParseSelectedTypeReferenceEntry(nlohmann::json::array()).has_value());
    EXPECT_FALSE(ParseSelectedTypeReferenceEntry(nlohmann::json{{"uri", "file:///tmp/a.cj"}}).has_value());
    EXPECT_FALSE(ParseSelectedTypeReferenceEntry(nlohmann::json{
        {"uri", "file:///tmp/a.cj"}, {"start", 1}, {"end", nlohmann::json{{"line", 1}, {"character", 2}}}
    }).has_value());
    EXPECT_FALSE(ParseSelectedTypeReferenceEntry(nlohmann::json{
        {"uri", "file:///tmp/a.cj"}, {"start", nlohmann::json{{"line", 1}}},
        {"end", nlohmann::json{{"line", 1}, {"character", 2}}}
    }).has_value());

    auto parsed = ParseSelectedTypeReferenceEntry(nlohmann::json{
        {"uri", "file:///tmp/a.cj"},
        {"start", nlohmann::json{{"line", 7}, {"character", 3}}},
        {"end", nlohmann::json{{"line", 7}, {"character", 9}}}
    });
    ASSERT_TRUE(parsed.has_value());
    EXPECT_FALSE(parsed->uri.file.empty());
    EXPECT_EQ(parsed->range.start.line, 7);
    EXPECT_EQ(parsed->range.start.column, 3);
    EXPECT_EQ(parsed->range.end.line, 7);
    EXPECT_EQ(parsed->range.end.column, 9);

    Location same = *parsed;
    EXPECT_TRUE(IsSameSelectedTypeReference(*parsed, same));
    same.range.end.column = 10;
    EXPECT_FALSE(IsSameSelectedTypeReference(*parsed, same));
}

TEST(ExtractInterfaceTest, NameReferenceRangeSupportsRefExprAndMemberAccess)
{
    RefExpr invalidRef;
    EXPECT_FALSE(GetNameReferenceRange(invalidRef).has_value());

    RefExpr ref;
    ref.ref.identifier = Cangjie::SrcIdentifier("radius", {1, 8, 5}, {1, 8, 11}, false);
    auto refRange = GetNameReferenceRange(ref);
    ASSERT_TRUE(refRange.has_value());
    EXPECT_EQ(refRange->start.line, 8);
    EXPECT_EQ(refRange->start.column, 5);
    EXPECT_EQ(refRange->end.column, 11);

    MemberAccess access;
    access.field = Cangjie::SrcIdentifier("base", {1, 9, 12}, {1, 9, 16}, false);
    auto accessRange = GetNameReferenceRange(access);
    ASSERT_TRUE(accessRange.has_value());
    EXPECT_EQ(accessRange->start.line, 9);
    EXPECT_EQ(accessRange->start.column, 12);
    EXPECT_EQ(accessRange->end.column, 16);
}

TEST(ExtractInterfaceTest, ImportInterfaceEditRejectsInvalidOrSameDirectoryInputs)
{
    File sourceFile;
    sourceFile.filePath = "D:/pkg/source/person.cj";
    sourceFile.begin = {1, 1, 1};

    EXPECT_FALSE(BuildImportInterfaceEditForFile(sourceFile, "", "Person").has_value());
    EXPECT_FALSE(BuildImportInterfaceEditForFile(sourceFile, "D:/pkg/target/person.cj", "").has_value());
    EXPECT_FALSE(BuildImportInterfaceEditForFile(sourceFile,
        "D:/pkg/source/person_interface.cj", "Person").has_value());
}
