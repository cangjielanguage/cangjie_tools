// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Utils.h"
#include "gtest/gtest.h"
#include <memory>
#include <string>
#include <vector>
#include "Options.h"

using namespace ark;

class StringType : public Ty {
public:
    explicit StringType(TypeKind k) : Ty(k) {}

    std::string String() const override
    {
        return "String";
    }

    TypeKind kind() const
    {
        return TypeKind::TYPE_CSTRING;
    }
};

class IntType : public Ty {
public:
    explicit IntType(TypeKind k) : Ty(k) {}

    std::string String() const override
    {
        return "Int";
    }

    TypeKind kind() const
    {
        return TypeKind::TYPE_UNIT;
    }
};

class GenericType : public Ty {
public:
    explicit GenericType(TypeKind k) : Ty(k) {}

    std::string String() const override
    {
        return "Generic";
    }

    TypeKind kind() const
    {
        return TypeKind::TYPE_GENERICS;
    }
};

class JStringType : public Ty {
public:
    explicit JStringType(TypeKind k) : Ty(k) {}

    std::string String() const override
    {
        return "JStringType";
    }

    bool IsJstring() const
    {
        return true;
    }
};

// --- CheckTypeCompatibility Tests ---
TEST(UtilsTest, UtilsTest001)
{
    const Ty* lvalue = nullptr;
    const Ty* rvalue = nullptr;
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest002)
{
    std::vector<Ptr<Ty>> emptyArgs;
    const Ty* lvalue = nullptr;
    const Ty* rvalue = std::make_shared<TupleTy>(emptyArgs).get();
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest003)
{
    std::vector<Ptr<Ty>> emptyArgs;
    const Ty* lvalue = std::make_shared<TupleTy>(emptyArgs).get();
    const Ty* rvalue = nullptr;
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest005)
{
    std::vector<Ptr<Ty>> args;
    args.emplace_back(Ptr<GenericType>(new GenericType(TypeKind::TYPE_GENERICS)));
    std::shared_ptr<TupleTy> ltuplePtr = std::make_shared<TupleTy>(args);
    std::shared_ptr<TupleTy> rtuplePtr = std::make_shared<TupleTy>(args);
    const Ty* lvalue = ltuplePtr.get();
    const Ty* rvalue = rtuplePtr.get();
    EXPECT_EQ(ark::TypeCompatibility::IDENTICAL, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest006)
{
    std::vector<Ptr<Ty>> largs;
    largs.emplace_back(new IntType(TypeKind::TYPE_UNIT));
    largs.emplace_back(new IntType(TypeKind::TYPE_UNIT));
    std::vector<Ptr<Ty>> rargs;
    rargs.emplace_back(new StringType(TypeKind::TYPE_CSTRING));
    rargs.emplace_back(new StringType(TypeKind::TYPE_CSTRING));
    std::shared_ptr<TupleTy> ltuplePtr = std::make_shared<TupleTy>(largs);
    std::shared_ptr<TupleTy> rtuplePtr = std::make_shared<TupleTy>(rargs);
    const Ty* lvalue = ltuplePtr.get();
    const Ty* rvalue = rtuplePtr.get();
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

// --- IsMatchingCompletion Tests ---
TEST(UtilsTest, UtilsTest011)
{
    std::string prefix = "";
    std::string completionName = "test";
    bool caseSensitive = true;
    EXPECT_EQ(true, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest014)
{
    std::string prefix = "Hello";
    std::string completionName = "hello";
    bool caseSensitive = false;
    EXPECT_EQ(true, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

// --- GetFilterText Tests ---
TEST(UtilsTest, UtilsTest021)
{
    int argc = 1;
    const char* argv[] = {"program"};
    ark::Options::GetInstance().Parse(argc, argv);
    MessageHeaderEndOfLine::SetIsDeveco(false);
    std::string name = "testName";
    std::string prefix = "prefix";
    std::string expected = prefix + "_" + name;
    std::string result = GetFilterText(name, prefix);
    EXPECT_EQ(expected, result);
}

// --- AST Node Range Tests ---
TEST(UtilsTest, UtilsTest026)
{
    Ptr<const Node> node = nullptr;
    ark::Range result = GetIdentifierRange(node);
    EXPECT_EQ(result.end.fileID, 0);
    EXPECT_EQ(result.end.line, 0);
    EXPECT_EQ(result.end.column, 0);
}

// --- GetCommentKind Tests ---
TEST(UtilsTest, UtilsTest034)
{
    std::string comment = "/** 这是一个文档注释 */";
    ark::CommentKind result = GetCommentKind(comment);
    EXPECT_EQ(result, ark::CommentKind::DOC_COMMENT);
}

// --- IsZeroPosition Tests ---
class FakeNode : public Node {
public:
    explicit FakeNode(int line, int column)
    {
        end.line = line;
        end.column = column;
    }
};

TEST(UtilsTest, UtilsTest061)
{
    Ptr<const Node> node(new FakeNode(0, 0));
    EXPECT_TRUE(IsZeroPosition(node));
}

// --- ValidExtendIncludeGenericParam Tests ---
class FakeDecl : public Cangjie::AST::Decl {
public:
    explicit FakeDecl(ASTKind kind) : Decl(kind) {}
};

TEST(UtilsTest, UtilsTest064)
{
    Ptr<const Decl> decl(new FakeDecl(ASTKind::CLASS_DECL));
    EXPECT_TRUE(ValidExtendIncludeGenericParam(decl));
}

// --- InterpolatedString Position Tests ---
class FakeNode4SetRang : public Cangjie::AST::Node {
public:
    explicit FakeNode4SetRang(int fileID, int bLine, int bCol, int eLine, int eCol)
    {
        begin.fileID = fileID;
        begin.line   = bLine;
        begin.column = bCol;
        end.fileID   = fileID;
        end.line     = eLine;
        end.column   = eCol;
    }
};

// --- Rename for Interpolated String Tests ---
class FakeNode4InterpolatedStrInRename : public AST::Node {
public:
    explicit FakeNode4InterpolatedStrInRename(const std::string &s, const Position &nodeBegin)
        : AST::Node(ASTKind::DECL), text_(s), begin_(nodeBegin) {}

    std::string ToString() const override
    {
        return text_;
    }

    Position GetBegin() const
    {
        return begin_;
    }

private:
    std::string text_;
    Position    begin_;
};

// --- FuncSignature Tests ---
struct FakeFuncTy : public Cangjie::AST::FuncTy {
    explicit FakeFuncTy(const std::vector<Ptr<Ty>> &params = {}, Ptr<Ty> ret = nullptr, const Config &cfg = {})
        : FuncTy(params, ret, cfg) {}
};

struct FakeFuncDecl : public Cangjie::AST::FuncDecl {
    explicit FakeFuncDecl(const std::string &id, Ptr<Cangjie::AST::Ty> t)
    {
        identifier = id;
        ty = std::move(t);
    }
};

// --- SearchContext Tests ---
TEST(UtilsTest, UtilsTest084)
{
    auto result = SearchContext(nullptr, "anything");
    EXPECT_TRUE(result.empty());
}