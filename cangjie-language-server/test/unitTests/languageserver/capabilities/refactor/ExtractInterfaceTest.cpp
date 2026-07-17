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
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace Cangjie::AST;

namespace ark {
class Tweak {
public:
    struct Effect {
        std::optional<std::string> showMessage;
        std::map<std::string, std::vector<TextEdit>> applyEdits;
        std::vector<nlohmann::json> documentChanges;
        bool format = true;
    };
};
std::string GetVisibility(const Cangjie::AST::Decl &decl);
std::string EscapeJsonString(const std::string &input);
std::optional<Location> ParseSelectedTypeReferenceEntry(const nlohmann::json &entry);
bool IsSameSelectedTypeReference(const Location &lhs, const Location &rhs);
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
bool IsTypeReferenceMatch(const Cangjie::AST::Type &type, const Cangjie::AST::Decl &targetDecl,
    const Range &referenceRange);
size_t FindHeaderClauseInsertOffset(const std::string &headerText);
struct InterfaceInfo {
    struct MemberMeta {
        std::string signature;
        bool isStatic = false;
        bool isMut = false;
        bool isOperator = false;
        bool isOverride = false;
        std::string visibility;
    };

    std::string name;
    std::string genericParams;
    std::string genericWhereClause;
    std::vector<std::string> inheritedTypes;
    std::vector<std::string> members;
    std::vector<MemberMeta> metas;
    std::unordered_map<std::string, std::string> implBodies;
    std::unordered_map<std::string, std::string> memberComments;
};
std::string BuildInterfaceMemberHeader(const std::string &member, const InterfaceInfo::MemberMeta *meta);
TextEdit InsertInterfaceDeclToTargetFile(const std::string &targetPath, const InterfaceInfo &info);
void AppendCreateFileDocumentChange(Tweak::Effect &effect, const std::string &targetPath, const TextEdit &edit);
} // namespace ark

using namespace ark;

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

TEST(ExtractInterfaceTest, TypeReferenceMatchCoversTargetAndRangeBranches)
{
    ClassDecl targetDecl;
    targetDecl.identifier = "Person";

    RefType noTargetType;
    noTargetType.ref.identifier = Cangjie::SrcIdentifier("Person", {1, 5, 3}, {1, 5, 9}, false);
    Range fullRange{{1, 5, 3}, {1, 5, 9}};
    EXPECT_FALSE(IsTypeReferenceMatch(noTargetType, targetDecl, fullRange));

    ClassDecl otherDecl;
    otherDecl.identifier = "Other";
    RefType otherType;
    otherType.ref.identifier = Cangjie::SrcIdentifier("Other", {1, 5, 3}, {1, 5, 8}, false);
    otherType.ref.target = Ptr<Decl>(&otherDecl);
    EXPECT_FALSE(IsTypeReferenceMatch(otherType, targetDecl, fullRange));

    RefType matchedType;
    matchedType.ref.identifier = Cangjie::SrcIdentifier("Person", {1, 5, 3}, {1, 5, 9}, false);
    matchedType.ref.target = Ptr<Decl>(&targetDecl);
    EXPECT_TRUE(IsTypeReferenceMatch(matchedType, targetDecl, fullRange));

    Range narrowRange{{1, 5, 4}, {1, 5, 8}};
    EXPECT_FALSE(IsTypeReferenceMatch(matchedType, targetDecl, narrowRange));

    RefType zeroPositionType;
    zeroPositionType.ref.identifier = Cangjie::SrcIdentifier("Person", {}, {}, false);
    zeroPositionType.ref.target = Ptr<Decl>(&targetDecl);
    EXPECT_FALSE(IsTypeReferenceMatch(zeroPositionType, targetDecl, fullRange));
}

TEST(ExtractInterfaceTest, HeaderClauseInsertOffsetCoversWhereBodyAndFallback)
{
    std::string headerWithWhere = "class Box<T>   where T <: ToString {";
    EXPECT_EQ(FindHeaderClauseInsertOffset(headerWithWhere), headerWithWhere.find("where") - 3);

    std::string headerWithBody = "class Person   {";
    EXPECT_EQ(FindHeaderClauseInsertOffset(headerWithBody), headerWithBody.find("{") - 3);

    std::string headerWithoutBody = "class Person";
    EXPECT_EQ(FindHeaderClauseInsertOffset(headerWithoutBody), headerWithoutBody.size());
}

TEST(ExtractInterfaceTest, InterfaceMemberHeaderCoversMetaCombinations)
{
    EXPECT_EQ(BuildInterfaceMemberHeader("printName(): String", nullptr), "    func printName(): String");

    InterfaceInfo::MemberMeta staticMeta;
    staticMeta.isStatic = true;
    EXPECT_EQ(BuildInterfaceMemberHeader("buildDefault(): Person", &staticMeta),
        "    static func buildDefault(): Person");

    InterfaceInfo::MemberMeta mutOperatorMeta;
    mutOperatorMeta.isMut = true;
    mutOperatorMeta.isOperator = true;
    EXPECT_EQ(BuildInterfaceMemberHeader("+ (rhs: Person): Person", &mutOperatorMeta),
        "    mut operator func + (rhs: Person): Person");

    InterfaceInfo::MemberMeta overrideMeta;
    overrideMeta.isOverride = true;
    EXPECT_EQ(BuildInterfaceMemberHeader("printName(): String", &overrideMeta),
        "    override func printName(): String");
}

TEST(ExtractInterfaceTest, AppendCreateFileDocumentChangeAddsCreateAndEditEntries)
{
    Tweak::Effect emptyTargetEffect;
    TextEdit edit{{Cangjie::Position(0, 0, 0), Cangjie::Position(0, 0, 0)}, "public interface Empty {}\n"};
    AppendCreateFileDocumentChange(emptyTargetEffect, "", edit);
    EXPECT_TRUE(emptyTargetEffect.documentChanges.empty());

    Tweak::Effect effect;
    AppendCreateFileDocumentChange(effect, "D:/pkg/demo/printable.cj", edit);

    ASSERT_EQ(effect.documentChanges.size(), 2);
    EXPECT_EQ(effect.documentChanges[0].at("kind"), "create");
    ASSERT_TRUE(effect.documentChanges[0].contains("uri"));
    EXPECT_NE(effect.documentChanges[0].at("uri").get<std::string>().find("printable.cj"), std::string::npos);

    ASSERT_TRUE(effect.documentChanges[1].contains("textDocument"));
    ASSERT_TRUE(effect.documentChanges[1].contains("edits"));
    EXPECT_EQ(effect.documentChanges[1].at("textDocument").at("version"), -1);
    EXPECT_NE(effect.documentChanges[1].at("textDocument").at("uri").get<std::string>().find("printable.cj"),
        std::string::npos);
    ASSERT_EQ(effect.documentChanges[1].at("edits").size(), 1);
    EXPECT_EQ(effect.documentChanges[1].at("edits")[0].at("newText"), edit.newText);
}
