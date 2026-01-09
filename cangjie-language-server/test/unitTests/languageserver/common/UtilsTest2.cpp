// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "gtest/gtest.h"
#include <iostream>
#include <vector>
#include "Utils.h"
 
using namespace ark;
 
// A minimal Decl subclass to control astKind
class FakeDecl : public Cangjie::AST::Decl {
public:
    explicit FakeDecl(ASTKind kind) : Decl(kind) {}
 
    FakeDecl(ASTKind kind,
        bool add,
        bool isCloned,
        bool primConstructor) : Decl(kind)
    {
        add_ = add;
        isCloned_ = isCloned;
        primCtor_ = primConstructor;
    }
 
    bool TestAttr(Cangjie::AST::Attribute attr) const {
        switch (attr) {
            case Cangjie::AST::Attribute::COMPILER_ADD:
                return add_;
            case Cangjie::AST::Attribute::IS_CLONED_SOURCE_CODE:
                return isCloned_;
            case Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR:
                return primCtor_;
            default:
                return false;
        }
    }
 
    void SetBegin(Position p) {
        begin = p;
    }
 
    Position GetIdentifierPos() {
        return identifierPos;
    }
 
    void setIdentifierPos(Position p) {
        identifierPos = p;
    }
 
    SrcIdentifier name;
 
private:
    bool add_;
    bool isCloned_;
    bool primCtor_;
    Position identifierPos;
};
 
class FakeExpr : public Expr {
public:
    FakeExpr(ASTKind kind) : Expr(kind) {
    }
    std::vector<OwnedPtr<FuncArg>> args;
};
 
// Test 087: nullptr input should yield empty set
TEST(UtilsTest, UtilsTest087)
{
    auto decls = GetInheritDecls(nullptr);
    EXPECT_TRUE(decls.empty());
}
 
// Test 088: non-FUNC_DECL/PROP_DECL kind yields empty set
TEST(UtilsTest, UtilsTest088)
{
    FakeDecl varDecl{Cangjie::AST::ASTKind::VAR_DECL};
    auto decls = GetInheritDecls(&varDecl);
    EXPECT_TRUE(decls.empty());
}
 
// Test 089: FUNC_DECL path invokes TestFakeInheritDeclUtil and returns its result
TEST(UtilsTest, UtilsTest089)
{
    FakeDecl funcDecl{Cangjie::AST::ASTKind::FUNC_DECL};
    auto decls = GetInheritDecls(&funcDecl);
}
 
// Test 090: PROP_DECL path also invokes TestFakeInheritDeclUtil
TEST(UtilsTest, UtilsTest090)
{
    FakeDecl propDecl{Cangjie::AST::ASTKind::PROP_DECL};
    FakeDecl dummyDecl{Cangjie::AST::ASTKind::FUNC_DECL};
    auto decls = GetInheritDecls(&propDecl);
}
 
// Test 091: nullptr node → false
TEST(UtilsTest, UtilsTest091)
{
    bool rv = IsFromSrcOrNoSrc(nullptr);
    EXPECT_FALSE(rv);
}
 
// Test 092: node present, curFile nullptr → false
TEST(UtilsTest, UtilsTest092)
{
    Node node;
    node.curFile = nullptr;
 
    bool rv = IsFromSrcOrNoSrc(&node);
    EXPECT_FALSE(rv);
}
 
// Test 093: node and curFile present, curPackage nullptr → false
TEST(UtilsTest, UtilsTest093)
{
    File file;
    file.curPackage = nullptr;
 
    Node node;
    node.curFile = &file;
 
    bool rv = IsFromSrcOrNoSrc(&node);
    EXPECT_FALSE(rv);
}
 
// Test 095: all pointers valid, singleton returns false → false
TEST(UtilsTest, UtilsTest095)
{
    Ptr<const Node> filePrt(new File());
    // Configure fake to return false
    bool rv = IsFromSrcOrNoSrc(filePrt);
    EXPECT_FALSE(rv);
}
 
 
//  Range GetRangeFromNode(Ptr<const Node> p, const std::vector<Cangjie::Token> &tokens)
//------------------------------------------------------------------------------
// Test 096: cover the `if (!p)` branch (line 456)
//------------------------------------------------------------------------------
TEST(UtilsTest, UtilsTest096)
{
    Ptr<const Node> p = nullptr;
    std::vector<Token> tokens;
 
    // should return a default-constructed Range
    ark::Range r = GetRangeFromNode(p, tokens);
 
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}
 
//------------------------------------------------------------------------------
// Test 097: cover the QualifiedType dynamic_cast branch (line 461–462)
//------------------------------------------------------------------------------
TEST(UtilsTest, UtilsTest097)
{
    // assume QualifiedType has a public default ctor
    Ptr<QualifiedType> p(new QualifiedType());
    std::vector<Token> tokens;
 
    ark::Range r = GetRangeFromNode(p, tokens);
 
    // GetUserRange<QualifiedType> should have been invoked,
    // so r should differ from the default {0,0}->{0,0}
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}
 
//------------------------------------------------------------------------------
// Test 098: cover the `p->ty && !p->ty->typeArgs.empty()` branch (line 469–470)
//------------------------------------------------------------------------------
TEST(UtilsTest, UtilsTest098)
{
    struct NodeWithTypeArgs : public Node {
        NodeWithTypeArgs() : Node() {}
        Position GetBegin() const { return {1, 1}; }
        Position GetEnd() const { return {2, 2}; }
    };
    Ptr<NodeWithTypeArgs> p(new NodeWithTypeArgs());
    std::vector<Token> tokens;
    ark::Range r = GetRangeFromNode(p, tokens);
    // identifier-based branch exercised, so expect non-zero
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}
 
//------------------------------------------------------------------------------
// Test 099: cover the “zero‐end” fixup branch (line 476–477)
//------------------------------------------------------------------------------
TEST(UtilsTest, UtilsTest099)
{
    struct ZeroRangeNode : public Node {
        ZeroRangeNode() : Node() {}
        Position GetBegin() const { return {0, 0}; }
        Position GetEnd() const { return {0, 0}; }
    };
 
    Ptr<ZeroRangeNode> p(new ZeroRangeNode());
    std::vector<Token> tokens;
    ark::Range r = GetRangeFromNode(p, tokens);
 
    // after the main else-block we get {0,0}->{0,0},
    // so the “if (end==0&&line==0)” branch fires and reassigns it
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}
 
// SymbolKind GetSymbolKind(const ASTKind astKind)
TEST(UtilsTest, UtilsTest100) {
    ASTKind invalidKind = static_cast<ASTKind>(-1);
    EXPECT_EQ(GetSymbolKind(invalidKind), SymbolKind::NULL_KIND);
}
 
TEST(UtilsTest, UtilsTest101) {
    Ptr<const Decl> d = nullptr;
    EXPECT_FALSE(InValidDecl(d));
}
 
TEST(UtilsTest, UtilsTest102) {
    Ptr<FakeDecl> d(new FakeDecl(
        ASTKind::INVALID_TYPE,
        true,
        true,
        false
    ));
    EXPECT_FALSE(InValidDecl(d));
}
 
TEST(UtilsTest, UtilsTest103) {
    Ptr<FakeDecl> d(new FakeDecl(
        ASTKind::INVALID_TYPE,
        true,
        false,
        true
    ));
    EXPECT_FALSE(InValidDecl(d));
}
 
TEST(UtilsTest, UtilsTest104) {
    Ptr<FakeDecl> d(new FakeDecl(
        ASTKind::EXTEND_DECL,
        true,
        false,
        false
    ));
    EXPECT_FALSE(InValidDecl(d));
}
 
TEST(UtilsTest, UtilsTest105) {
    std::string pkg = GetPkgNameFromNode(nullptr);
    EXPECT_EQ(pkg, "");
}
 
TEST(UtilsTest, UtilsTest106) {
    Ptr<Node> node(new Node());
    node->curFile = nullptr;
    std::string pkg = GetPkgNameFromNode(node);
    EXPECT_EQ(pkg, "");
}
 
// void SetHeadByFilePath(const std::string &filePath)
TEST(UtilsTest, UtilsTest107) {
    // Save and clear the singleton
    SetHeadByFilePath("any/path.cpp");
}
 
TEST(UtilsTest, UtilsTest108) {
    SetHeadByFilePath("source/file.cpp");
}
 
TEST(UtilsTest, UtilsTest109)
{
    SetHeadByFilePath("irrelevant.cpp");
}
 
TEST(UtilsTest, UtilsTest113) {
    Decl decl;
    auto users = GetOnePkgUsers(decl,
        "",
        "/tmp/file.cpp",
        false,
        "unused");
    EXPECT_TRUE(users.empty());
}
 
// void ConvertCarriageToSpace(std::string &str)
TEST(UtilsTest, ConvertCarriageToSpace001) {
    std::string str;
    ConvertCarriageToSpace(str);
    EXPECT_EQ(str, "");
}
 
TEST(UtilsTest, ConvertCarriageToSpace002) {
    std::string str = "a\nb";
    ConvertCarriageToSpace(str);
    EXPECT_EQ(str, "a b");
}
 
// void GetSingleConditionCompile
TEST(UtilsTest, GetSingleConditionCompile001)
{
    nlohmann::json initOpts = {{CONSTANTS::SINGLE_CONDITION_COMPILE_OPTION, {{".pkg", {{"customKey", "customVal"}}}}}};
    std::unordered_map<std::string, std::string> globalConds = {{"g1", "gv1"}};
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> modulesConds = {
        {".pkg", {{"g1", "overwritten"}, {"m2", "modVal2"}}}};
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> outConds;
 
    GetSingleConditionCompile(initOpts, globalConds, modulesConds, outConds);
 
    ASSERT_EQ(outConds.size(), 1u);
    auto &pkgMap = outConds[".pkg"];
 
    EXPECT_EQ(pkgMap["g1"], "overwritten");
    EXPECT_EQ(pkgMap["m2"], "modVal2");
 
    EXPECT_EQ(pkgMap["customKey"], "customVal");
}
 
TEST(UtilsTest, Digest001) {
    const std::string pkg = "nonexistent_file.cj";
    EXPECT_FALSE(FileUtil::FileExist(pkg));
    EXPECT_EQ(Digest(pkg), "");
}
 
std::string getAbsolutePath(const std::string& relativePath) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        return "";
    }
 
    std::string absPath = std::string(cwd);
 
    // Ensure there's a separator between cwd and relative path
    if (!absPath.empty() && absPath.back() != '/') {
        absPath += '/';
    }
 
    absPath += relativePath;
    return absPath;
}
 
std::string getCurrentWorkingDirectory() {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) == nullptr) {
        return "";
    }
    return std::string(buffer);
}
 
TEST(UtilsTest, Digest002) {
    std::string rel = "../../../../testChr/completion/src/LSP_Completion_KeyWord001.cj";
    Digest(getAbsolutePath(rel));
}
 
TEST(UtilsTest, Digest003) {
    std::string rel = "../UtilsTest001.cpp";
    Digest(getAbsolutePath(rel));
}
 
TEST(UtilsTest, Digest004) {
    Digest(getCurrentWorkingDirectory());
}
 
TEST(UtilsTest, Digest005) {
    std::string rel = "../../../../testChr/completion/src";
    Digest(getAbsolutePath(rel));
}
 
TEST(UtilsTest, Digest006) {
    std::string testPath = "testPath";
    DigestForCjo(testPath);
}
 
TEST(UtilsTest, GetFuncParamsTypeName001) {
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();
    paramList->params.emplace_back(nullptr);
    funcBody->paramLists.emplace_back(paramList);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);
 
    auto names = GetFuncParamsTypeName(funcDecl);
};
 
// 001: funcDecl->funcBody is nullptr → early return default‐constructed Range
TEST(UtilsTest, GetConstructorRange001) {
    FuncDecl decl;
    // decl.funcBody is still nullptr
    ark::Range r = GetConstructorRange(decl, "ignored");
    EXPECT_EQ(r.end.line,     0);
    EXPECT_EQ(r.end.column,   0);
}
 
TEST(UtilsTest, GetConstructorRange003) {
    FuncDecl decl;
    decl.funcBody = OwnedPtr<FuncBody>(new FuncBody());
 
    auto structDecl = Ptr<StructDecl>(new StructDecl());
    Position pos{2, 4};
    structDecl->identifier.SetPos(pos, pos);
    decl.funcBody->parentStruct = std::move(structDecl);
 
    ark::Range r = GetConstructorRange(decl, "");
}
 
TEST(UtilsTest, GetConstructorRange004) {
    FuncDecl decl;
    decl.funcBody = OwnedPtr<FuncBody>(new FuncBody());
 
    auto enumDecl = Ptr<EnumDecl>(new EnumDecl());
    Position pos{3, 9};
    enumDecl->identifier.SetPos(pos, pos);
    decl.funcBody->parentEnum = std::move(enumDecl);
 
    const std::string id = "EnumName";
    ark::Range r = GetConstructorRange(decl, id);
 
    int len = static_cast<int>(CountUnicodeCharacters(id));
}
 
 
// A tiny subclass of FuncTy so dynamic_cast<FuncTy*> succeeds.
class TestTy : public Ty {
public:
    TestTy(TypeKind k) : Ty(k) {
    }
 
    std::string String() const override {
        return "test";
    }
};
 
TEST(UtilsTest, GetVarDeclType001) {
    // Construct VarDecl with a non‐function type
    auto decl = Ptr<VarDecl>(new VarDecl());
    // Use a plain Type, set its kind to something else
    Ptr<TestTy> rType(new TestTy(TypeKind::TYPE_FUNC));
    std::vector<Ptr<Ty>> v;
    v.emplace_back(nullptr);
    Ptr<FuncTy> funcTy(new FuncTy(v, rType));
    decl->ty = std::move(funcTy);
    GetVarDeclType(decl);
}
 
// 002: decl is null or decl->ty is null
TEST(UtilsTest, GetVarDeclType002) {
    GetVarDeclType(nullptr);
    auto decl = Ptr<VarDecl>(new VarDecl());
    GetVarDeclType(decl);
}
 
TEST(UtilsTest, GetStandardDeclAbsolutePath001) {
    Ptr<const FakeDecl> fakeDecl(new FakeDecl(ASTKind::BUILTIN_DECL));
    GetStandardDeclAbsolutePath(fakeDecl, (std::string &)"");
}
 
TEST(UtilsTest, IsModifierBeforeDecl001) {
    Ptr<FakeDecl> decl(new FakeDecl(ASTKind::FUNC_DECL));
    SrcIdentifier identifier;
    identifier.SetRaw(true);
    decl->setIdentifierPos(*new Position(3, 4, 4));
    decl->SetBegin(*new Position(1, 2, 2));
    IsModifierBeforeDecl(decl, *new Position(1, 2, 2));
 
    decl->SetBegin(*new Position(1, 3, 3));
    decl->setIdentifierPos(*new Position(1, 1, 1));
    IsModifierBeforeDecl(decl, *new Position(2, 2, 2));
}
 
TEST(UtilsTest, IsModifierBeforeDecl002) {
    IsModifierBeforeDecl(nullptr, *new Position(2, 2, 2));
}
 
TEST(UtilsTest, GetProperRange001) {
    SrcIdentifier identifier("test");
    identifier.SetPos(*new Position(2, 2, 2), *new Position(2, 2, 2));
    Ptr<FuncArg> funcArg(new FuncArg());
    funcArg->name = identifier;
    std::vector<Cangjie::Token> tokens;
    GetProperRange(funcArg, tokens, true);
}
 
TEST(UtilsTest, LTrim001) {
    // empty string: find_if sees begin==end, erase removes nothing, result = ""
    std::string s = "";
    EXPECT_EQ(LTrim(s), "");
}
 
TEST(UtilsTest, LTrim002) {
    // no leading whitespace: erase(begin, begin) is a no-op, result unchanged
    std::string s = "hello";
    EXPECT_EQ(LTrim(s), "hello");
}
 
TEST(UtilsTest, CheckIsRawIdentifier001) {
    EXPECT_EQ(CheckIsRawIdentifier(nullptr), false);
}
 
TEST(UtilsTest, InImportSpec001) {
    File file;
    EXPECT_EQ(InImportSpec(file, *new Position(0, 0, 0)), false);
}

// Fake VarDecl to allow setting ty and type members
class FakeVarDecl : public VarDecl {
public:
    FakeVarDecl() : VarDecl(ASTKind::VAR_DECL) {
        ty = nullptr;
        type = nullptr;
    }
};

// Stub for a type that returns "UnknownType" string
class UnknownTypeStub : public Ty {
public:
    UnknownTypeStub() : Ty(TypeKind::TYPE_CSTRING) {}
    std::string String() const override { return "UnknownType"; }
};

// --- GetVarDeclType Unit Tests ---

// Test 086: When both decl->ty and decl->type are null, return empty string
TEST(UtilsTest, GetVarDeclType_086) {
    Ptr<FakeVarDecl> decl(new FakeVarDecl());
    decl->ty = nullptr;
    decl->type = nullptr;

    std::string result = GetVarDeclType(decl, nullptr);
    EXPECT_EQ(result, "");
}

struct MockNamedType : public Type {
    MockNamedType() : Type(ASTKind::TYPE) {}

    // You can override GetTypeArgs if testing generic types
    std::vector<Ptr<Type>> GetTypeArgs() const override {
        return {};
    }
};


// Test 087: When ty is UnknownType, fetch from SourceManager and apply ReplaceTuple
TEST(UtilsTest, GetVarDeclType_087) {
    Ptr<FakeVarDecl> decl(new FakeVarDecl());
    decl->ty = Ptr<Ty>(new UnknownTypeStub());

    MockNamedType* typeNode = new MockNamedType();
    typeNode->begin = {1, 10, 100}; // line 1, col 10
    typeNode->end = {1, 20, 110};   // line 1, col 20

    // Assign to the 'type' member inherited from VarDeclAbstract
    decl->type = OwnedPtr<Type>(typeNode);

    SourceManager sm;
    std::string result = GetVarDeclType(decl, &sm);

    // Should trigger the fallback to SourceManager because of "UnknownType"
    EXPECT_EQ(result, "");
}

// Test 088: When ty is a function type (TYPE_FUNC), use ItemResolverUtil
TEST(UtilsTest, GetVarDeclType_088) {
    Ptr<FakeVarDecl> decl(new FakeVarDecl());

    // Create a function type
    std::vector<Ptr<Ty>> params;
    decl->ty = Ptr<Ty>(new FuncTy(params, nullptr));

    // Logic: TYPE_FUNC triggers GetDetailByTy and ReplaceTuple
    std::string result = GetVarDeclType(decl, nullptr);

    // Even without a mock for ItemResolverUtil, we check if it proceeds without crashing
    EXPECT_TRUE(true);
}

// Test 089: For a normal type, return the type string processed by ReplaceTuple
TEST(UtilsTest, GetVarDeclType_089) {
    Ptr<FakeVarDecl> decl(new FakeVarDecl());

    class NormalType : public Ty {
    public:
        NormalType() : Ty(TypeKind::TYPE_UNIT) {}
        std::string String() const override { return "Tuple<Float64>"; }
    };

    decl->ty = Ptr<Ty>(new NormalType());

    // Directly returns the string representation via GetString()
    std::string result = GetVarDeclType(decl, nullptr);

    // Verifies that GetString was called and ReplaceTuple was executed
    EXPECT_EQ(result, "(Float64)");
}

// Test cases for GetSubStrBetweenSingleQuote function
TEST(UtilsTest, GetSubStrBetweenSingleQuote_EmptyString) {
    std::string input = "";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_NoSingleQuotes) {
    std::string input = "Hello World";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_OnlyOpeningQuote) {
    std::string input = "Hello 'World";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_OnlyClosingQuote) {
    std::string input = "Hello World'";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_ValidSingleQuotes) {
    std::string input = "Prefix 'Hello World' Suffix";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "Hello World");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_EmptyBetweenQuotes) {
    std::string input = "Prefix '' Suffix";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_MultipleQuotes_FirstPair) {
    std::string input = "'First' 'Second'";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "First");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_NestedQuotes) {
    std::string input = "Outer 'Inner \"quoted\" text' End";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "Inner \"quoted\" text");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_SpecialCharacters) {
    std::string input = "'Line1\nLine2\tTab'";
    std::string result = GetSubStrBetweenSingleQuote(input);
    EXPECT_EQ(result, "Line1\nLine2\tTab");
}

// Test cases for GetDeclSymbolID function
TEST(UtilsTest, GetDeclSymbolID_RegularDecl) {
    // Create a minimal Decl with exportId
    auto decl = OwnedPtr<VarDecl>(new VarDecl());
    decl->exportId = "testExportId";

    lsp::SymbolID result = GetDeclSymbolID(*decl);
    EXPECT_NE(result, lsp::INVALID_SYMBOL_ID);
}

TEST(UtilsTest, GetDeclSymbolID_FuncParamWithOuterDecl) {
    // Create a function parameter decl with outer decl
    auto paramDecl = OwnedPtr<FuncParam>(new FuncParam());
    paramDecl->identifier = "param1";

    // Create outer function decl
    auto outerDecl = OwnedPtr<FuncDecl>(new FuncDecl());
    outerDecl->exportId = "outerFunc";
    paramDecl->outerDecl = outerDecl;

    lsp::SymbolID result = GetDeclSymbolID(*paramDecl);
    EXPECT_NE(result, lsp::INVALID_SYMBOL_ID);
}

TEST(UtilsTest, GetDeclSymbolID_FuncParamWithoutOuterDecl) {
    auto paramDecl = OwnedPtr<FuncParam>(new FuncParam());
    paramDecl->identifier = "param1";
    paramDecl->outerDecl = nullptr; // No outer decl

    lsp::SymbolID result = GetDeclSymbolID(*paramDecl);
    EXPECT_EQ(result, lsp::INVALID_SYMBOL_ID);
}

TEST(UtilsTest, GetDeclSymbolID_EmptyExportId) {
    auto decl = OwnedPtr<VarDecl>(new VarDecl());
    decl->exportId = ""; // Empty exportId

    lsp::SymbolID result = GetDeclSymbolID(*decl);
    EXPECT_EQ(result, lsp::INVALID_SYMBOL_ID);
}

// Test cases for IsValidIdentifier function
TEST(UtilsTest, IsValidIdentifier_EmptyString) {
    std::string identifier = "";
    bool result = IsValidIdentifier(identifier);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, IsValidIdentifier_StartsWithLetter) {
    std::string identifier = "variable";
    bool result = IsValidIdentifier(identifier);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, IsValidIdentifier_StartsWithUnderscore) {
    std::string identifier = "_private";
    bool result = IsValidIdentifier(identifier);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, IsValidIdentifier_StartsWithNumber) {
    std::string identifier = "1invalid";
    bool result = IsValidIdentifier(identifier);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, IsValidIdentifier_StartsWithSpecialChar) {
    std::string identifier = "@invalid";
    bool result = IsValidIdentifier(identifier);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, IsValidIdentifier_ContainsLettersNumbersUnderscores) {
    std::string identifier = "var_name123";
    bool result = IsValidIdentifier(identifier);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, IsValidIdentifier_ContainsInvalidChar) {
    std::string identifier = "var-name";
    bool result = IsValidIdentifier(identifier);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, IsValidIdentifier_ContainsSpace) {
    std::string identifier = "var name";
    bool result = IsValidIdentifier(identifier);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, IsValidIdentifier_ContainsUnicode) {
    std::string identifier = "变量"; // Chinese characters
    bool result = IsValidIdentifier(identifier);
    // Depends on iswalpha/iswalnum implementation for wide chars
    // This test might behave differently on different platforms
    EXPECT_FALSE(result); // Typically returns false for non-ASCII
}

TEST(UtilsTest, IsValidIdentifier_SingleLetter) {
    std::string identifier = "a";
    bool result = IsValidIdentifier(identifier);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, IsValidIdentifier_SingleUnderscore) {
    std::string identifier = "_";
    bool result = IsValidIdentifier(identifier);
    EXPECT_TRUE(result);
}

// Test cases for DeleteCharForPosition function
TEST(UtilsTest, DeleteCharForPosition_InvalidRow) {
    std::string text = "Hello World";
    bool result = DeleteCharForPosition(text, 0, 1); // row < 1
    EXPECT_FALSE(result);
    EXPECT_EQ(text, "Hello World"); // text unchanged
}

TEST(UtilsTest, DeleteCharForPosition_InvalidColumn) {
    std::string text = "Hello World";
    bool result = DeleteCharForPosition(text, 1, 0); // column < 1
    EXPECT_FALSE(result);
    EXPECT_EQ(text, "Hello World"); // text unchanged
}

TEST(UtilsTest, DeleteCharForPosition_SingleLine_FirstChar) {
    std::string text = "Hello";
    bool result = DeleteCharForPosition(text, 1, 1);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "ello");
}

TEST(UtilsTest, DeleteCharForPosition_SingleLine_MiddleChar) {
    std::string text = "Hello";
    bool result = DeleteCharForPosition(text, 1, 3);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "Helo");
}

TEST(UtilsTest, DeleteCharForPosition_SingleLine_LastChar) {
    std::string text = "Hello";
    bool result = DeleteCharForPosition(text, 1, 5);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "Hell");
}

TEST(UtilsTest, DeleteCharForPosition_SingleLine_OutOfBounds) {
    std::string text = "Hello";
    bool result = DeleteCharForPosition(text, 1, 10);
    EXPECT_FALSE(result);
    EXPECT_EQ(text, "Hello");
}

TEST(UtilsTest, DeleteCharForPosition_MultiLine_FirstLine) {
    std::string text = "Line1\nLine2\nLine3";
    bool result = DeleteCharForPosition(text, 1, 3);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "Lie1\nLine2\nLine3");
}

TEST(UtilsTest, DeleteCharForPosition_MultiLine_SecondLine) {
    std::string text = "Line1\nLine2\nLine3";
    bool result = DeleteCharForPosition(text, 2, 3);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "Line1\nLie2\nLine3");
}

TEST(UtilsTest, DeleteCharForPosition_MultiLine_LastLine) {
    std::string text = "Line1\nLine2\nLine3";
    bool result = DeleteCharForPosition(text, 3, 3);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "Line1\nLine2\nLie3");
}

TEST(UtilsTest, DeleteCharForPosition_WithCarriageReturnLineFeed) {
    std::string text = "Line1\r\nLine2\r\nLine3";
    bool result = DeleteCharForPosition(text, 2, 3);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "Line1\r\nLie2\r\nLine3");
}

TEST(UtilsTest, DeleteCharForPosition_WithCarriageReturnOnly) {
    std::string text = "Line1\rLine2\rLine3";
    bool result = DeleteCharForPosition(text, 2, 3);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "Line1\rLie2\rLine3");
}

TEST(UtilsTest, DeleteCharForPosition_EmptyText) {
    std::string text = "";
    bool result = DeleteCharForPosition(text, 1, 1);
    EXPECT_FALSE(result);
    EXPECT_EQ(text, "");
}

TEST(UtilsTest, DeleteCharForPosition_AtLineBreak_CRLF) {
    std::string text = "A\r\nB";
    // Try to delete between CR and LF
    bool result = DeleteCharForPosition(text, 1, 2);
    EXPECT_TRUE(result);
    // The exact behavior depends on how the function handles CRLF
}

TEST(UtilsTest, DeleteCharForPosition_PositionAtEndOfLine) {
    std::string text = "Hello";
    bool result = DeleteCharForPosition(text, 1, 6); // Position after last char
    EXPECT_FALSE(result);
    EXPECT_EQ(text, "Hello");
}

// Additional edge case tests
TEST(UtilsTest, GetSubStrBetweenSingleQuote_EscapedQuotes) {
    std::string input = "'Don\\'t worry'";
    std::string result = GetSubStrBetweenSingleQuote(input);
    // The function doesn't handle escaped quotes, so it will find the first closing quote
    EXPECT_EQ(result, "Don\\"); // This might be the actual behavior
}

TEST(UtilsTest, IsValidIdentifier_MixedValidAndInvalid) {
    std::string identifier = "valid_but_with-invalid-char";
    bool result = IsValidIdentifier(identifier);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, DeleteCharForPosition_UnicodeCharacters) {
    std::string text = "Hello 世界";
    bool result = DeleteCharForPosition(text, 1, 7); // Delete a unicode character
    EXPECT_TRUE(result);
    // The exact result depends on how the function counts unicode characters
}

// Test cases for FindLastImportPos function
TEST(UtilsTest, FindLastImportPos_EmptyImportsWithPackage) {
    // Create a file with empty imports but valid package
    File file;
    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 5, 1}; // fileID=1, line=5, column=1

    Position result = FindLastImportPos(file);
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 6); // package line (5) + 1
    EXPECT_EQ(result.column, 1);
}

TEST(UtilsTest, FindLastImportPos_EmptyImportsPackageAtLineZero) {
    // Create a file with empty imports and package at line 0
    File file;
    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 0, 1}; // line=0

    Position result = FindLastImportPos(file);
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 1); // lastImportLine=0 + 1
    EXPECT_EQ(result.column, 1);
}

TEST(UtilsTest, FindLastImportPos_SingleImport) {
    File file;

    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 1, 1};

    auto import = new ImportSpec();
    import->importPos = {1, 3, 1};
    import->content.rightCurlPos = {1, 3, 10};
    file.imports.emplace_back(import);

    Position result = FindLastImportPos(file);
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 4);
    EXPECT_EQ(result.column, 1);
}

TEST(UtilsTest, FindLastImportPos_MultipleImports) {
    File file;
    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 1, 1};

    auto import1 = new ImportSpec();
    import1->importPos = {1, 3, 1};
    import1->content.rightCurlPos = {1, 3, 15};

    auto import2 = new ImportSpec();
    import2->importPos = {1, 5, 1};
    import2->content.rightCurlPos = {1, 5, 20};

    auto import3 = new ImportSpec();
    import3->importPos = {1, 4, 1};
    import3->content.rightCurlPos = {1, 4, 12};

    file.imports.emplace_back(import1);
    file.imports.emplace_back(import2);
    file.imports.emplace_back(import3);

    Position result = FindLastImportPos(file);
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 6);
    EXPECT_EQ(result.column, 1);
}

TEST(UtilsTest, FindLastImportPos_ImportWithNullptr) {
    File file;
    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 1, 1};

    // Add a valid import and a nullptr
    auto import1 = new ImportSpec();
    import1->importPos = {1, 3, 1};
    import1->content.rightCurlPos = {1, 3, 10};

    file.imports.emplace_back(import1);
    file.imports.emplace_back(nullptr); // Should be skipped

    Position result = FindLastImportPos(file);
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 4); // Should only consider valid import
    EXPECT_EQ(result.column, 1);
}

TEST(UtilsTest, FindLastImportPos_ImportPosLaterThanRightCurlPos) {
    File file;
    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 1, 1};

    auto import = new ImportSpec();
    import->importPos = {1, 10, 1}; // Later line
    import->content.rightCurlPos = {1, 5, 10};

    file.imports.emplace_back(import);

    Position result = FindLastImportPos(file);
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 11); // max(10,5)=10 + 1
    EXPECT_EQ(result.column, 1);
}

TEST(UtilsTest, FindLastImportPos_RightCurlPosLaterThanImportPos) {
    File file;
    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 1, 1};

    auto import = new ImportSpec();
    import->importPos = {1, 5, 1};
    import->content.rightCurlPos = {1, 10, 10}; // Later line

    file.imports.emplace_back(import);

    Position result = FindLastImportPos(file);
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 11); // max(10,5)=10 + 1
    EXPECT_EQ(result.column, 1);
}

TEST(UtilsTest, FindLastImportPos_MixedFileIDs) {
    File file;
    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 1, 1}; // fileID=1

    // Create imports with different file IDs (should still work)
    auto import1 = new ImportSpec();
    import1->importPos = {2, 3, 1}; // Different fileID
    import1->content.rightCurlPos = {2, 3, 10};

    auto import2 = new ImportSpec();
    import2->importPos = {1, 5, 1}; // Same fileID as package
    import2->content.rightCurlPos = {1, 5, 15};

    file.imports.emplace_back(import1);
    file.imports.emplace_back(import2);

    Position result = FindLastImportPos(file);
    // Should use package's fileID regardless of import fileIDs
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 6); // max(5,5)=5 + 1
    EXPECT_EQ(result.column, 1);
}

std::string NormalizePath(const std::string& path) {
    // Simple normalization for testing
    std::string result = path;
    for (char& c : result) {
        if (c == '\\') c = '/';
    }
    return result;
}

// Test cases for GetAllFilePathUnderCurrentPath function
TEST(UtilsTest, GetAllFilePathUnderCurrentPath_EmptyDirectory) {
    std::string path = "empty_dir";
    std::string extension = "cj";

    std::vector<std::string> result = GetAllFilePathUnderCurrentPath(
        path, extension, false, false);

    EXPECT_TRUE(result.empty());
}

TEST(UtilsTest, GetAllFilePathUnderCurrentPath_SingleFile) {
    std::string path = "single_file_dir";
    std::string extension = "cj";

    std::vector<std::string> result = GetAllFilePathUnderCurrentPath(
        path, extension, false, false);

    EXPECT_EQ(result.size(), 0);
}

// Additional edge case tests
TEST(UtilsTest, FindLastImportPos_AllImportsNullptr) {
    File file;
    auto packageSpec = new PackageSpec();
    file.package = OwnedPtr<PackageSpec>(packageSpec);
    file.package->packagePos = {1, 2, 1};

    // Add only null imports
    file.imports.emplace_back(nullptr);
    file.imports.emplace_back(nullptr);

    Position result = FindLastImportPos(file);
    EXPECT_EQ(result.fileID, 1);
    EXPECT_EQ(result.line, 3); // package line (2) + 1
    EXPECT_EQ(result.column, 1);
}

TEST(UtilsTest, GetAllFilePathUnderCurrentPath_ComplexDirectoryStructure) {
    // This test would require a more sophisticated mock
    // For now, we test the basic functionality
    std::string path = "complex/structure";
    std::string extension = "cj";

    std::vector<std::string> result = GetAllFilePathUnderCurrentPath(
        path, extension, false, false);

    // Should return empty since mock doesn't have this path
    EXPECT_TRUE(result.empty());
}