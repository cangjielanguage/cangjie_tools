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

    FakeDecl(ASTKind kind, bool add, bool isCloned, bool primConstructor) : Decl(kind)
    {
        add_ = add;
        isCloned_ = isCloned;
        primCtor_ = primConstructor;
    }

    bool TestAttr(Cangjie::AST::Attribute attr) const
    {
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

    void SetBegin(Position p)
    {
        begin = p;
    }

    Position GetIdentifierPos()
    {
        return identifierPos;
    }

    void setIdentifierPos(Position p)
    {
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
    explicit FakeExpr(ASTKind kind) : Expr(kind) {}
    std::vector<OwnedPtr<FuncArg>> args;
};

// --- Tests ---

TEST(UtilsTest, UtilsTest087)
{
    auto decls = GetInheritDecls(nullptr);
    EXPECT_TRUE(decls.empty());
}

TEST(UtilsTest, UtilsTest088)
{
    FakeDecl varDecl{Cangjie::AST::ASTKind::VAR_DECL};
    auto decls = GetInheritDecls(&varDecl);
    EXPECT_TRUE(decls.empty());
}

TEST(UtilsTest, UtilsTest089)
{
    FakeDecl funcDecl{Cangjie::AST::ASTKind::FUNC_DECL};
    auto decls = GetInheritDecls(&funcDecl);
}

TEST(UtilsTest, UtilsTest091)
{
    bool rv = IsFromSrcOrNoSrc(nullptr);
    EXPECT_FALSE(rv);
}

TEST(UtilsTest, UtilsTest092)
{
    Node node;
    node.curFile = nullptr;
    bool rv = IsFromSrcOrNoSrc(&node);
    EXPECT_FALSE(rv);
}

TEST(UtilsTest, UtilsTest096)
{
    Ptr<const Node> p = nullptr;
    std::vector<Token> tokens;
    ark::Range r = GetRangeFromNode(p, tokens);
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}

TEST(UtilsTest, UtilsTest100)
{
    ASTKind invalidKind = static_cast<ASTKind>(-1);
    EXPECT_EQ(GetSymbolKind(invalidKind), SymbolKind::NULL_KIND);
}

TEST(UtilsTest, UtilsTest105)
{
    std::string pkg = GetPkgNameFromNode(nullptr);
    EXPECT_EQ(pkg, "");
}

TEST(UtilsTest, ConvertCarriageToSpace002)
{
    std::string str = "a\nb";
    ConvertCarriageToSpace(str);
    EXPECT_EQ(str, "a b");
}

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
    EXPECT_EQ(pkgMap["customKey"], "customVal");
}

class TestTy : public Ty {
public:
    explicit TestTy(TypeKind k) : Ty(k) {}
    std::string String() const override
    {
        return "test";
    }
};

TEST(UtilsTest, GetVarDeclType001)
{
    auto decl = Ptr<VarDecl>(new VarDecl());
    Ptr<TestTy> rType(new TestTy(TypeKind::TYPE_FUNC));
    std::vector<Ptr<Ty>> v;
    v.emplace_back(nullptr);
    Ptr<FuncTy> funcTy(new FuncTy(v, rType));
    decl->ty = std::move(funcTy);
    GetVarDeclType(decl);
}

TEST(UtilsTest, LTrim002)
{
    std::string s = "hello";
    EXPECT_EQ(LTrim(s), "hello");
}

class FakeVarDecl : public VarDecl {
public:
    FakeVarDecl() : VarDecl(ASTKind::VAR_DECL)
    {
        ty = nullptr;
        type = nullptr;
    }
};

class UnknownTypeStub : public Ty {
public:
    UnknownTypeStub() : Ty(TypeKind::TYPE_CSTRING) {}
    std::string String() const override
    {
        return "UnknownType";
    }
};

TEST(UtilsTest, GetSubStrBetweenSingleQuote_ValidSingleQuotes)
{
    std::string input = "Prefix 'Hello World' Suffix";
    EXPECT_EQ(GetSubStrBetweenSingleQuote(input), "Hello World");
}

TEST(UtilsTest, IsValidIdentifier_StartsWithNumber)
{
    std::string identifier = "1invalid";
    EXPECT_FALSE(IsValidIdentifier(identifier));
}

TEST(UtilsTest, DeleteCharForPosition_SingleLine_MiddleChar)
{
    std::string text = "Hello";
    bool result = DeleteCharForPosition(text, 1, 3);
    EXPECT_TRUE(result);
    EXPECT_EQ(text, "Helo");
}