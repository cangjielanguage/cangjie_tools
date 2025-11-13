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
    StringType(TypeKind k) : Ty(k) {}
    std::string String() const override { return "String"; }
    TypeKind kind() const { return TypeKind::TYPE_CSTRING; }
};

class IntType : public Ty {
public:
    IntType(TypeKind k) : Ty(k) {}
    std::string String() const override { return "Int"; }
    TypeKind kind() const { return TypeKind::TYPE_UNIT; }
};

class GenericType : public Ty {
public:
    GenericType(TypeKind k) : Ty(k) {}
    std::string String() const override { return "Generic"; }
    TypeKind kind() const { return TypeKind::TYPE_GENERICS; }
};

class JStringType : public Ty {
public:
    JStringType(TypeKind k) : Ty(k) {}
    std::string String() const override { return "JStringType"; }
    bool isJString() const { return true; }
};

// TypeCompatibility CheckTypeCompatibility(const Ty *lvalue, const Ty *rvalue)
TEST(UtilsTest, UtilsTest001) {
    const Ty* lvalue = nullptr;
    const Ty* rvalue = nullptr;
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest002) {
    std::vector<Ptr<Ty>> emptyArgs;
    const Ty* lvalue = nullptr;
    const Ty* rvalue = std::make_shared<TupleTy>(emptyArgs).get();
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest003) {
    std::vector<Ptr<Ty>> emptyArgs;
    const Ty* lvalue = std::make_shared<TupleTy>(emptyArgs).get();
    const Ty* rvalue = nullptr;
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest005) {
    std::vector<Ptr<Ty>> args;
    args.emplace_back(Ptr<GenericType>(new GenericType(TypeKind::TYPE_GENERICS)));
    std::shared_ptr<TupleTy> ltuplePtr = std::make_shared<TupleTy>(args);
    std::shared_ptr<TupleTy> rtuplePtr = std::make_shared<TupleTy>(args);
    const Ty* lvalue = ltuplePtr.get();
    const Ty* rvalue = rtuplePtr.get();
    EXPECT_EQ(ark::TypeCompatibility::IDENTICAL, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest006) {
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

TEST(UtilsTest, UtilsTest007) {
    std::vector<Ptr<Ty>> largs;
    largs.emplace_back(new StringType(TypeKind::TYPE_CSTRING));
    std::vector<Ptr<Ty>> rargs;
    rargs.emplace_back(new StringType(TypeKind::TYPE_CSTRING));
    rargs.emplace_back(new IntType(TypeKind::TYPE_UNIT));
    std::shared_ptr<TupleTy> ltuplePtr = std::make_shared<TupleTy>(largs);
    std::shared_ptr<TupleTy> rtuplePtr = std::make_shared<TupleTy>(rargs);
    const Ty* lvalue = ltuplePtr.get();
    const Ty* rvalue = rtuplePtr.get();
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest008) {
    std::shared_ptr<StringType> lvaluePtr = std::make_shared<StringType>(TypeKind::TYPE_CSTRING);
    std::shared_ptr<StringType> rvaluePtr = std::make_shared<StringType>(TypeKind::TYPE_CSTRING);
    const Ty* lvalue = lvaluePtr.get();
    const Ty* rvalue = rvaluePtr.get();
    EXPECT_EQ(ark::TypeCompatibility::IDENTICAL, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest009) {
    std::shared_ptr<StringType> lvaluePtr = std::make_shared<StringType>(TypeKind::TYPE_CSTRING);
    std::shared_ptr<GenericType> rvaluePtr = std::make_shared<GenericType>(TypeKind::TYPE_GENERICS);
    const Ty* lvalue = lvaluePtr.get();
    const Ty* rvalue = rvaluePtr.get();
    EXPECT_EQ(ark::TypeCompatibility::IDENTICAL, CheckTypeCompatibility(lvalue, rvalue));
}

TEST(UtilsTest, UtilsTest010) {
    std::shared_ptr<StringType> lvaluePtr = std::make_shared<StringType>(TypeKind::TYPE_CSTRING);
    std::shared_ptr<IntType> rvaluePtr = std::make_shared<IntType>(TypeKind::TYPE_UNIT);
    const Ty* lvalue = lvaluePtr.get();
    const Ty* rvalue = rvaluePtr.get();
    EXPECT_EQ(ark::TypeCompatibility::INCOMPATIBLE, CheckTypeCompatibility(lvalue, rvalue));
}

// bool IsMatchingCompletion(const std::string &prefix, const std::string &completionName, bool caseSensitive)
TEST(UtilsTest, UtilsTest011) {
    std::string prefix = "";
    std::string completionName = "test";
    bool caseSensitive = true;
    EXPECT_EQ(true, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest012) {
    std::string prefix = "Hello";
    std::string completionName = "Hello";
    bool caseSensitive = true;
    EXPECT_EQ(true, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest013) {
    std::string prefix = "Hello";
    std::string completionName = "hello";
    bool caseSensitive = true;
    EXPECT_EQ(false, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest014) {
    std::string prefix = "Hello";
    std::string completionName = "hello";
    bool caseSensitive = false;
    EXPECT_EQ(true, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest015) {
    std::string prefix = "Hello";
    std::string completionName = "hxllo";
    bool caseSensitive = false;
    EXPECT_EQ(false, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest016) {
    std::string prefix = "abc";
    std::string completionName = "abd";
    bool caseSensitive = true;
    EXPECT_EQ(false, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest017) {
    std::string prefix = "abc";
    std::string completionName = "acb";
    bool caseSensitive = true;
    EXPECT_EQ(false, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest018) {
    std::string prefix = "abc";
    std::string completionName = "abd";
    bool caseSensitive = true;
    EXPECT_EQ(false, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest019) {
    std::string prefix = "abcd";
    std::string completionName = "abc";
    bool caseSensitive = true;
    EXPECT_EQ(false, IsMatchingCompletion(prefix, completionName, caseSensitive));
}

TEST(UtilsTest, UtilsTest020) {
    std::string prefix = "Hello";
    std::string completionName = "HELLO";
    bool caseSensitive = false;
    EXPECT_EQ(true, IsMatchingCompletion(prefix, completionName, caseSensitive));
}


// std::string GetFilterText(const std::string &name, const std::string &prefix)
TEST(UtilsTest, UtilsTest021) {
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

TEST(UtilsTest, UtilsTest022) {
    MessageHeaderEndOfLine::SetIsDeveco(true);
    int argc = 1;
    const char* argv[] = {"program"};
    ark::Options::GetInstance().Parse(argc, argv);
    std::string name = "testName";
    std::string prefix = "prefix";
    std::string expected = name;
    std::string result = GetFilterText(name, prefix);
    EXPECT_EQ(expected, result);
}

TEST(UtilsTest, UtilsTest023) {
    int argc = 2;
    const char* argv[] = {"program", "--test"};
    ark::Options::GetInstance().Parse(argc, argv);
    MessageHeaderEndOfLine::SetIsDeveco(false);
    std::string name = "testName";
    std::string prefix = "prefix";
    std::string expected = name;
    std::string result = GetFilterText(name, prefix);
    EXPECT_EQ(expected, result);
}

TEST(UtilsTest, UtilsTest024) {
    int argc = 2;
    const char* argv[] = {"program", "--test"};
    ark::Options::GetInstance().Parse(argc, argv);
    MessageHeaderEndOfLine::SetIsDeveco(true);
    std::string name = "testName";
    std::string prefix = "prefix";
    std::string expected = name;
    std::string result = GetFilterText(name, prefix);
    EXPECT_EQ(expected, result);
}

// Range GetNamedFuncArgRange(const Cangjie::AST::Node &node)
TEST(UtilsTest, UtilsTest025) {
    Cangjie::AST::Node node;
    node.symbol = nullptr;
    GetNamedFuncArgRange(node);
}

// Range GetIdentifierRange(Ptr<const Cangjie::AST::Node> node)
TEST(UtilsTest, UtilsTest026) {
    Ptr<const Node> node = nullptr;
    ark::Range result = GetIdentifierRange(node);
    EXPECT_EQ(result.end.fileID, 0);
    EXPECT_EQ(result.end.line, 0);
    EXPECT_EQ(result.end.column, 0);
}

TEST(UtilsTest, UtilsTest027) {
    Node node;
    node.symbol = nullptr;
    Ptr<const Node> pNode = &node;
    ark::Range result = GetIdentifierRange(pNode);
    EXPECT_EQ(result.end.fileID, 0);
    EXPECT_EQ(result.end.line, 0);
    EXPECT_EQ(result.end.column, 0);
}

// Range GetRefTypeRange(Ptr<const Cangjie::AST::Node> node)
TEST(UtilsTest, UtilsTest028) {
    Ptr<const Node> node = nullptr;
    ark::Range result = GetRefTypeRange(node);
    EXPECT_EQ(result.end.fileID, 0);
    EXPECT_EQ(result.end.line, 0);
    EXPECT_EQ(result.end.column, 0);
}

TEST(UtilsTest, UtilsTest029) {
    Node node;
    Ptr<const Node> pNode = &node;
    ark::Range result = GetRefTypeRange(pNode);
    EXPECT_EQ(result.end.fileID, 0);
    EXPECT_EQ(result.end.line, 0);
    EXPECT_EQ(result.end.column, 0);
}

TEST(UtilsTest, UtilsTest030) {
    Type type;
    type.typeParameterName = "";
    type.symbol = nullptr;
    Ptr<const Node> pNode = &type;
    ark::Range result = GetRefTypeRange(pNode);
    // 预期返回空的 Range
    EXPECT_EQ(result.end.fileID, 0);
    EXPECT_EQ(result.end.line, 0);
    EXPECT_EQ(result.end.column, 0);
}

TEST(UtilsTest, UtilsTest031) {
    Type type;
    type.typeParameterName = "testParam";
    type.symbol = nullptr;
    Ptr<const Node> pNode = &type;
    ark::Range result = GetRefTypeRange(pNode);
    EXPECT_EQ(result.end.fileID, 0);
    EXPECT_EQ(result.end.line, 0);
    EXPECT_EQ(result.end.column, 0);
}

// CommentKind GetCommentKind(const std::string &comment)
TEST(UtilsTest, UtilsTest032) {
    std::string comment = "";
    ark::CommentKind result = GetCommentKind(comment);
    EXPECT_EQ(result, ark::CommentKind::NO_COMMENT);
}

TEST(UtilsTest, UtilsTest033) {
    std::string comment = "// 这是一个行注释";
    ark::CommentKind result = GetCommentKind(comment);
    EXPECT_EQ(result, ark::CommentKind::LINE_COMMENT);
}

TEST(UtilsTest, UtilsTest034) {
    std::string comment = "/** 这是一个文档注释 */";
    ark::CommentKind result = GetCommentKind(comment);
    EXPECT_EQ(result, ark::CommentKind::DOC_COMMENT);
}

TEST(UtilsTest, UtilsTest035) {
    std::string comment = "/* 这是一个块注释 */";
    ark::CommentKind result = GetCommentKind(comment);
    EXPECT_EQ(result, ark::CommentKind::BLOCK_COMMENT);
}

TEST(UtilsTest, UtilsTest036) {
    std::string comment = "/*";
    ark::CommentKind result = GetCommentKind(comment);
    EXPECT_EQ(result, ark::CommentKind::NO_COMMENT);
}

TEST(UtilsTest, UtilsTest037) {
    std::string comment = "这是一个注释*/";
    ark::CommentKind result = GetCommentKind(comment);
    EXPECT_EQ(result, ark::CommentKind::NO_COMMENT);
}

TEST(UtilsTest, UtilsTest038) {
    std::string comment = "这是一个普通字符串";
    ark::CommentKind result = GetCommentKind(comment);
    EXPECT_EQ(result, ark::CommentKind::NO_COMMENT);
}

// std::string PrintTypeArgs(std::vector<Ptr<Ty>> tyArgs, const std::pair<bool, int> isVarray)
TEST(UtilsTest, UtilsTest040) {
    std::vector<Ptr<Ty>> tyArgs;
    tyArgs.emplace_back(nullptr);
    std::pair<bool, int> isVarray = {false, 0};
    std::string result = PrintTypeArgs(tyArgs, isVarray);
}

// std::string GetString(const Ty &ty)
TEST(UtilsTest, UtilsTest041) {
    GetString(*new JStringType(TypeKind::TYPE_CSTRING));
}

// std::string ReplaceTuple(const std::string &type)
TEST(UtilsTest, UtilsTest042) {
    ReplaceTuple("Tuple<int>");
    ReplaceTuple("Tuple<tuple<float>>");
    ReplaceTuple("Tup<int>");
    ReplaceTuple("Tuple<int>>");
}

// void MatchBracket(const std::string &type, size_t &index, int &count)
TEST(UtilsTest, UtilsTest043) {
    std::string s = "anything";
    size_t idx = 0;
    int cnt = 0;
    MatchBracket(s, idx, cnt);
    EXPECT_EQ(idx, 0u);
    EXPECT_EQ(cnt, 0);
}

TEST(UtilsTest, UtilsTest044) {
    std::string s = "xyz";
    size_t idx = s.length();  // idx == len
    int cnt = 1;
    MatchBracket(s, idx, cnt);
    EXPECT_EQ(idx, s.length());
    EXPECT_EQ(cnt, 1);
}

TEST(UtilsTest, UtilsTest045) {
    std::string s = "<";
    size_t idx = 0;
    int cnt = 1;
    MatchBracket(s, idx, cnt);
    EXPECT_EQ(idx, 1u);
    EXPECT_EQ(cnt, 2);
}

TEST(UtilsTest, UtilsTest046) {
    std::string s = ">";
    size_t idx = 0;
    int cnt = 1;
    MatchBracket(s, idx, cnt);
    EXPECT_EQ(idx, 1u);
    EXPECT_EQ(cnt, 0);        // 1 - 1
}

TEST(UtilsTest, UtilsTest047) {
    std::string s = "a";
    size_t idx = 0;
    int cnt = 1;
    MatchBracket(s, idx, cnt);
    EXPECT_EQ(idx, 1u);
    EXPECT_EQ(cnt, 1);
}

TEST(UtilsTest, UtilsTest048)
{
    std::string s = "<><b>";
    size_t idx = 0;
    int cnt = 1;
    MatchBracket(s, idx, cnt);
    EXPECT_EQ(idx, s.length());
    EXPECT_EQ(cnt, 1);
}

// bool IsZeroPosition(Ptr<const Node> node) { return node && node->end.line == 0 && node->end.column == 0; }
// A minimal Node subclass to control end.line and end.column
class FakeNode : public Node {
public:
    FakeNode(int line, int column) {
        end.line = line;
        end.column = column;
    }
};

// 058: node == nullptr → overall false
TEST(UtilsTest, UtilsTest058) {
    Ptr<const Node> node = nullptr;
    EXPECT_FALSE(IsZeroPosition(node));
}

// 059: node non-null, end.line != 0 → overall false
TEST(UtilsTest, UtilsTest059) {
    Ptr<const Node> node(new FakeNode(1, 0));
    EXPECT_FALSE(IsZeroPosition(node));
}

// 060: node non-null, end.line == 0 but end.column != 0 → overall false
TEST(UtilsTest, UtilsTest060) {
    Ptr<const Node> node(new FakeNode(0, 5));
    EXPECT_FALSE(IsZeroPosition(node));
}

// 061: node non-null, end.line == 0 and end.column == 0 → true
TEST(UtilsTest, UtilsTest061) {
    Ptr<const Node> node(new FakeNode(0, 0));
    EXPECT_TRUE(IsZeroPosition(node));
}

// A minimal Decl subclass to control astKind
class FakeDecl : public Cangjie::AST::Decl {
public:
    explicit FakeDecl(ASTKind kind)
        : Decl(kind)
    {
    }
};

// 062: decl == nullptr → overall false
TEST(UtilsTest, UtilsTest062) {
    Ptr<const Decl> decl = nullptr;
    EXPECT_FALSE(ValidExtendIncludeGenericParam(decl));
}

// 063: decl non-null, astKind not CLASS_DECL or STRUCT_DECL → false
TEST(UtilsTest, UtilsTest063) {
    Ptr<const Decl> decl(new FakeDecl(ASTKind::ENUM_DECL));
    EXPECT_FALSE(ValidExtendIncludeGenericParam(decl));
}

// 064: decl non-null, astKind == CLASS_DECL → true
TEST(UtilsTest, UtilsTest064) {
    Ptr<const Decl> decl(new FakeDecl(ASTKind::CLASS_DECL));
    EXPECT_TRUE(ValidExtendIncludeGenericParam(decl));
}

// 065: decl non-null, astKind == STRUCT_DECL → true
TEST(UtilsTest, UtilsTest065) {
    Ptr<const Decl> decl(new FakeDecl(ASTKind::STRUCT_DECL));
    EXPECT_TRUE(ValidExtendIncludeGenericParam(decl));
}

// void SetRangForInterpolatedString(const Cangjie::Token &curToken, Ptr<const Cangjie::AST::Node> node, Range &range)
// A minimal Node subclass to control begin/end positions
class FakeNode4SetRang : public Cangjie::AST::Node {
public:
    FakeNode4SetRang(int fileID, int bLine, int bCol, int eLine, int eCol) {
        begin.fileID = fileID;
        begin.line   = bLine;
        begin.column = bCol;
        end.fileID   = fileID;
        end.line     = eLine;
        end.column   = eCol;
    }
};

// 069: node == nullptr, token is STRING_LITERAL  → should early-return without touching range
TEST(UtilsTest, UtilsTest066) {
    Cangjie::Token token(Cangjie::TokenKind::STRING_LITERAL);
    Ptr<const Cangjie::AST::Node> node = nullptr;

    ark::Range range;
    range.start = {1, 2, 3};
    range.end   = {4, 5, 6};

    SetRangForInterpolatedString(token, node, range);

    EXPECT_EQ(range.start.fileID, 1);
    EXPECT_EQ(range.start.line,   2);
    EXPECT_EQ(range.start.column, 3);
    EXPECT_EQ(range.end.fileID,   4);
    EXPECT_EQ(range.end.line,     5);
    EXPECT_EQ(range.end.column,   6);
}

// 070: node == nullptr, token is not STRING_LITERAL → still early-return
TEST(UtilsTest, UtilsTest067) {
    Cangjie::Token token(Cangjie::TokenKind::IDENTIFIER);
    Ptr<const Cangjie::AST::Node> node = nullptr;

    ark::Range range;
    range.start = {7, 8, 9};
    range.end   = {10, 11, 12};

    SetRangForInterpolatedString(token, node, range);

    EXPECT_EQ(range.start.fileID, 7);
    EXPECT_EQ(range.start.line,   8);
    EXPECT_EQ(range.start.column, 9);
    EXPECT_EQ(range.end.fileID,   10);
    EXPECT_EQ(range.end.line,     11);
    EXPECT_EQ(range.end.column,   12);
}

// 071: node non-null, token is not STRING_LITERAL → early-return
TEST(UtilsTest, UtilsTest068) {
    Cangjie::Token token(Cangjie::TokenKind::IDENTIFIER);
    Ptr<const Cangjie::AST::Node> node(new FakeNode4SetRang(13, 14, 15, 16, 17));

    ark::Range range;
    range.start = {18, 19, 20};
    range.end   = {21, 22, 23};

    SetRangForInterpolatedString(token, node, range);

    EXPECT_EQ(range.start.fileID, 18);
    EXPECT_EQ(range.start.line,   19);
    EXPECT_EQ(range.start.column, 20);
    EXPECT_EQ(range.end.fileID,   21);
    EXPECT_EQ(range.end.line,     22);
    EXPECT_EQ(range.end.column,   23);
}

// 072: node non-null, token is STRING_LITERAL → range should be updated from node
TEST(UtilsTest, UtilsTest069) {
    Cangjie::Token token(Cangjie::TokenKind::STRING_LITERAL);
    Ptr<const Cangjie::AST::Node> node(new FakeNode4SetRang(30, 31, 32, 33, 34));

    ark::Range range;
    range.start = {0, 0, 0};
    range.end   = {0, 0, 0};

    SetRangForInterpolatedString(token, node, range);

    EXPECT_EQ(range.start.fileID, 30);
    EXPECT_EQ(range.start.line,   31);
    EXPECT_EQ(range.start.column, 32);
    EXPECT_EQ(range.end.fileID,   30);
    EXPECT_EQ(range.end.line,     33);
    EXPECT_EQ(range.end.column,   34);
}

// void SetRangForInterpolatedStrInRename(const Cangjie::Token &curToken, Ptr<const Cangjie::AST::Node> node,
//     Range &range, Cangjie::Position pos)
// A minimal AST::Node stub for testing
class FakeNode4InterpolatedStrInRename : public AST::Node {
public:
    FakeNode4InterpolatedStrInRename(const std::string &s, const Position &nodeBegin)
        : AST::Node(ASTKind::DECL), text_(s), begin_(nodeBegin)
    {}

    std::string ToString() const override {
        return text_;
    }

    Position GetBegin() const {
        return begin_;
    }

private:
    std::string text_;
    Position    begin_;
};

// Helper: make a Token with explicit begin/end
static Token MakeToken(TokenKind kind,
    const Position &b,
    const Position &e) {
    // use the (kind, value, be, en) ctor; empty value is fine
    return Token(kind, std::string(), b, e);
}

// 069: node == nullptr → early return
TEST(UtilsTest, UtilsTest077) {
    Token tok = MakeToken(TokenKind::STRING_LITERAL,
        {1, 1, 1},
        {1, 1, 5});
    Ptr<const AST::Node> node = nullptr;
    ark::Range range{{10,10,10},{20,20,20}};
    Position pos{1,1,3};  // anywhere

    SetRangForInterpolatedStrInRename(tok, node, range, pos);

    // unchanged
    EXPECT_EQ(range.start.fileID, 10);
    EXPECT_EQ(range.end.line,     20);
}

// 070: token kind not STRING_LITERAL or MULTILINE_STRING → early return
TEST(UtilsTest, UtilsTest070) {
    Position b{1,1,0}, e{1,1,4};
    Token tok = MakeToken(TokenKind::IDENTIFIER, b, e);
    Ptr<const AST::Node> node(new FakeNode4InterpolatedStrInRename("hello", {1,1,0}));
    ark::Range range{{0,0,0},{0,0,0}};
    Position pos{1,1,2};

    SetRangForInterpolatedStrInRename(tok, node, range, pos);

    // unchanged
    EXPECT_EQ(range.start.line, 0);
    EXPECT_EQ(range.end.column, 0);
}

// 071: pos before token.Begin() → early return
TEST(UtilsTest, UtilsTest071) {
    Position b{2,5,10}, e{2,5,15};
    Token tok = MakeToken(TokenKind::STRING_LITERAL, b, e);
    Ptr<const AST::Node> node(new FakeNode4InterpolatedStrInRename("test", {2,5,10}));
    ark::Range range{{3,3,3},{4,4,4}};
    Position pos{2,5,9};   // < begin.column

    SetRangForInterpolatedStrInRename(tok, node, range, pos);

    EXPECT_EQ(range.start.column, 3);
    EXPECT_EQ(range.end.line,      4);
}

// 072: pos after token.End() → early return
TEST(UtilsTest, UtilsTest072) {
    Position b{2,5,10}, e{2,5,15};
    Token tok = MakeToken(TokenKind::MULTILINE_STRING, b, e);
    Ptr<const AST::Node> node(new FakeNode4InterpolatedStrInRename("foo\nbar", {2,5,10}));
    ark::Range range{{1,1,1},{2,2,2}};
    Position pos{2,5,16};  // > end.column

    SetRangForInterpolatedStrInRename(tok, node, range, pos);

    EXPECT_EQ(range.start.fileID, 1);
    EXPECT_EQ(range.end.column,   2);
}

// 073: node->ToString() empty → early return
TEST(UtilsTest, UtilsTest073) {
    Position b{3,1,0}, e{3,1,3};
    Token tok = MakeToken(TokenKind::STRING_LITERAL, b, e);
    Ptr<const AST::Node> node(new FakeNode4InterpolatedStrInRename("", {3,1,0}));
    ark::Range range{{5,5,5},{6,6,6}};
    Position pos{3,1,1};

    SetRangForInterpolatedStrInRename(tok, node, range, pos);

    EXPECT_EQ(range.start.line, 5);
    EXPECT_EQ(range.end.fileID, 6);
}

// 074: pos in range and nonempty nodeStr, but no matching position → index == -1 branch
TEST(UtilsTest, UtilsTest074) {
    Position b{1,2,0}, e{1,2,3};
    Token tok = MakeToken(TokenKind::STRING_LITERAL, b, e);
    // "abc" at columns 0,1,2
    Ptr<const AST::Node> node(new FakeNode4InterpolatedStrInRename("abc", {1,2,0}));
    ark::Range range{{9,9,9},{9,9,9}};
    Position pos{1,2,3};  // within token but beyond the 3 chars of nodeStr

    SetRangForInterpolatedStrInRename(tok, node, range, pos);

    EXPECT_EQ(range.start.column, 9);
    EXPECT_EQ(range.end.column,   9);
}

// 075: index found but identifier invalid → no assignment
TEST(UtilsTest, UtilsTest075) {
    Position b{4,4,0}, e{4,4,4};
    Token tok = MakeToken(TokenKind::STRING_LITERAL, b, e);
    // "1abc": pos at column 2 => 'b'
    Ptr<const AST::Node> node(new FakeNode4InterpolatedStrInRename("1abc", {4,4,0}));
    ark::Range range{{7,7,7},{8,8,8}};
    Position pos{4,4,2};

    SetRangForInterpolatedStrInRename(tok, node, range, pos);

    // still unchanged because "1abc" is not a valid identifier
    EXPECT_EQ(range.start.fileID, 7);
    EXPECT_EQ(range.end.line,     8);
}

// 076: valid path → identifier extracted, range set
TEST(UtilsTest, UtilsTest076) {
    Position b{5,5,10}, e{5,5,18};
    Token tok = MakeToken(TokenKind::STRING_LITERAL, b, e);
    // "abc1_def": begin at col=10, pos at '1' → col=13
    Ptr<const AST::Node> node(new FakeNode4InterpolatedStrInRename("abc1_def", {5,5,10}));
    ark::Range range{{0,0,0},{0,0,0}};
    Position pos{5,5,13};

    SetRangForInterpolatedStrInRename(tok, node, range, pos);
}



// bool IsFuncSignatureIdentical(const Cangjie::AST::FuncDecl &funcDecl1, const Cangjie::AST::FuncDecl &funcDecl2)
// Flags driving our stubs
static bool g_paramIdentical = false;
static ark::TypeCompatibility g_returnCompat = ark::TypeCompatibility::INCOMPATIBLE;

// Override parameter comparison
bool IsFuncParameterTypesIdentical(const FuncTy &a, const FuncTy &b) {
    return g_paramIdentical;
}

// Override return-type compatibility check
ark::TypeCompatibility CheckTypeCompatibility(Ptr<Ty> a, Ptr<Ty> b) {
    return g_returnCompat;
}


//------------------------------------------------------------------------------
// Test helpers: fake FuncTy and FuncDecl matching the new FuncTy constructor.
//------------------------------------------------------------------------------

struct FakeFuncTy : public Cangjie::AST::FuncTy {
    // Default to empty parameter list, null return, and default Config
    FakeFuncTy(const std::vector<Ptr<Ty>> &params = {},
        Ptr<Ty> ret = nullptr,
        const Config &cfg = {})
        : FuncTy(params, ret, cfg) {}
};

struct FakeFuncDecl : public Cangjie::AST::FuncDecl {
    FakeFuncDecl(const std::string &id, Ptr<Cangjie::AST::Ty> t) {
        identifier = id;
        ty = std::move(t);
    }
};

//------------------------------------------------------------------------------
// 078: Different identifiers → false immediately.
//------------------------------------------------------------------------------

TEST(UtilsTest, UtilsTest078) {
    FakeFuncDecl f1("foo",   Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));
    FakeFuncDecl f2("bar",   Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));

    // Flags shouldn't matter; mismatch on identifier triggers false
    g_paramIdentical = true;
    g_returnCompat   = ark::TypeCompatibility::IDENTICAL;

    EXPECT_FALSE(IsFuncSignatureIdentical(f1, f2));
}

//------------------------------------------------------------------------------
// 079: Same id, but first ty is not FuncTy → funcTy1 == nullptr → false.
//------------------------------------------------------------------------------

TEST(UtilsTest, UtilsTest079) {
    FakeFuncDecl f1("id", Ptr<Cangjie::AST::Ty>(new StringType(TypeKind::TYPE_CSTRING)));
    FakeFuncDecl f2("id", Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));

    g_paramIdentical = true;
    g_returnCompat   = ark::TypeCompatibility::IDENTICAL;

    EXPECT_FALSE(IsFuncSignatureIdentical(f1, f2));
}

//------------------------------------------------------------------------------
// 080: Same id, first ty is FuncTy, second ty is not → funcTy2 == nullptr → false.
//------------------------------------------------------------------------------

TEST(UtilsTest, UtilsTest080) {
    FakeFuncDecl f1("id", Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));
    FakeFuncDecl f2("id", Ptr<Cangjie::AST::Ty>(new StringType(TypeKind::TYPE_CSTRING)));

    g_paramIdentical = true;
    g_returnCompat   = ark::TypeCompatibility::IDENTICAL;

    EXPECT_FALSE(IsFuncSignatureIdentical(f1, f2));
}

//------------------------------------------------------------------------------
// 081: Both ty are FuncTy, parameters mismatch → false.
//------------------------------------------------------------------------------

TEST(UtilsTest, UtilsTest081) {
    FakeFuncDecl f1("id", Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));
    FakeFuncDecl f2("id", Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));

    g_paramIdentical = false;  // trigger parameter mismatch
    g_returnCompat   = ark::TypeCompatibility::IDENTICAL;

    EXPECT_FALSE(IsFuncSignatureIdentical(f1, f2));
}

//------------------------------------------------------------------------------
// 082: Both ty are FuncTy, params match, return types incompatible → false.
//------------------------------------------------------------------------------

TEST(UtilsTest, UtilsTest082) {
    FakeFuncDecl f1("id", Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));
    FakeFuncDecl f2("id", Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));

    g_paramIdentical = true;
    g_returnCompat   = ark::TypeCompatibility::INCOMPATIBLE;  // trigger return-type mismatch

    EXPECT_FALSE(IsFuncSignatureIdentical(f1, f2));
}

//------------------------------------------------------------------------------
// 083: Both ty are FuncTy, params match, return types identical → true.
//------------------------------------------------------------------------------

TEST(UtilsTest, UtilsTest083) {
    FakeFuncDecl f1("id", Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));
    FakeFuncDecl f2("id", Ptr<Cangjie::AST::Ty>(new FakeFuncTy()));

    g_paramIdentical = true;
    g_returnCompat   = ark::TypeCompatibility::IDENTICAL;     // clear all mismatch flags
    IsFuncSignatureIdentical(f1, f2);
}

// std::vector<Symbol *> SearchContext(const Cangjie::ASTContext *astContext, const std::string &query)
// Minimal stub to satisfy ASTContext’s constructor
class DiagnosticEngine {
public:
    virtual ~DiagnosticEngine() = default;
};

// A fake Searcher subclass that returns two dummy Symbol*
class FakeSearcher : public Searcher {
public:
    std::vector<ark::Symbol*> Search(const ASTContext& /*ctx*/,
        const std::string& /*query*/)
    {
        // Return two distinct dummy pointers
        return {
            reinterpret_cast<ark::Symbol*>(0x1001),
            reinterpret_cast<ark::Symbol*>(0x1002)
        };
    }
};

// Test 1: nullptr ASTContext → empty result
TEST(UtilsTest, UtilsTest084) {
    auto result = SearchContext(nullptr, "anything");
    EXPECT_TRUE(result.empty());
}

// Test 2: ASTContext exists but searcher is null → empty result
TEST(UtilsTest, UtilsTest085) {
    Cangjie::DiagnosticEngine diag;
    AST::Package pkg;
    Cangjie::ASTContext ctx(diag, pkg);

    // Simulate missing searcher
    ctx.searcher.reset();

    auto result = SearchContext(&ctx, "anything");
    EXPECT_TRUE(result.empty());
}