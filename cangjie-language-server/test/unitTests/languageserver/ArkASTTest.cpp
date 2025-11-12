// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "ArkAST.cpp"
#include <string>
#include <vector>

using namespace ark;
using namespace Cangjie::AST;

auto CreateArkAST()
{
    // 1. Prepare the parameters required by the constructor
    std::pair<std::string, std::string> paths = {"file_path.cj", "let x = 10;"};
    Ptr<const File> node = nullptr; // Or point to a valid File object
    DiagnosticEngine diagEngine;    // Need a DiagnosticEngine instance
    PackageInstance* pkgInstance = nullptr; // Or point to a valid PackageInstance object
    SourceManager* sm = nullptr;    // Or point to a valid SourceManager object

    // 2. Construct ArkAST variable
    ArkAST ast(paths, node, diagEngine, pkgInstance, sm);
    return ast;
}

// Test CheckTokenKind function
TEST(ArkASTTest, CheckTokenKind) {
    ArkAST ast = CreateArkAST();

    // Test identifier
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::IDENTIFIER, false));

    // Test string literal
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::STRING_LITERAL, false));

    // Test multiline string
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::MULTILINE_STRING, false));

    // Test dollar identifier
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::DOLLAR_IDENTIFIER, false));

    // Test keywords
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::INIT, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::SUPER, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::THIS, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::MAIN, false));

    // Test modifiers
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::PUBLIC, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::PRIVATE, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::PROTECTED, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::OVERRIDE, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::ABSTRACT, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::OPEN, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::REDEF, false));
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::SEALED, false));

    // Test operator overloading
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::LSQUARE, false)); // "["
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::RSQUARE, false)); // "]"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::NOT, false));     // "!"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::EXP, false));     // "**"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::MUL, false));     // "*"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::MOD, false));     // "%"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::DIV, false));     // "/"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::ADD, false));     // "+"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::SUB, false));     // "-"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::LSHIFT, false));  // "<<"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::RSHIFT, false));  // ">>"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::LT, false));      // "<"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::LE, false));      // "<="
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::GT, false));      // ">"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::GE, false));      // ">="
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::EQUAL, false));   // "=="
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::NOTEQ, false));   // "!="
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::BITAND, false));  // "&"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::BITXOR, false));  // "^"
    EXPECT_TRUE(ast.CheckTokenKind(TokenKind::BITOR, false));   // "|"

    // Test unsupported token types
    EXPECT_FALSE(ast.CheckTokenKind(TokenKind::END, false));
    EXPECT_FALSE(ast.CheckTokenKind(TokenKind::ILLEGAL, false));
}

// Test CheckTokenKindWhenRenamed function
TEST(ArkASTTest, CheckTokenKindWhenRenamed) {
    ArkAST ast = CreateArkAST();

    // Test supported token types when renaming
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::IDENTIFIER));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::STRING_LITERAL));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::MULTILINE_STRING));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::DOLLAR_IDENTIFIER));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::PUBLIC));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::PRIVATE));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::PROTECTED));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::OVERRIDE));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::ABSTRACT));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::OPEN));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::REDEF));
    EXPECT_TRUE(ast.CheckTokenKindWhenRenamed(TokenKind::SEALED));

    // Test unsupported token types when renaming
    EXPECT_FALSE(ast.CheckTokenKindWhenRenamed(TokenKind::END));
    EXPECT_FALSE(ast.CheckTokenKindWhenRenamed(TokenKind::ILLEGAL));
}

// Test GetCurToken function
TEST(ArkASTTest, GetCurToken) {
    ArkAST ast = CreateArkAST();

    // Test empty tokens case
    Cangjie::Position pos{0, 1, 1};
    EXPECT_EQ(ast.GetCurToken(pos, 0, -1), -1); // Invalid range
    EXPECT_EQ(ast.GetCurToken(pos, -1, 0), -1); // Invalid start index
    EXPECT_EQ(ast.GetCurToken(pos, 0, 0), -1);  // Empty range
}

// Test GetCurTokenSkipSpace function
TEST(ArkASTTest, GetCurTokenSkipSpace) {
    ArkAST ast = CreateArkAST();

    // Test empty tokens case
    Cangjie::Position pos{0, 1, 1};
    EXPECT_EQ(ast.GetCurTokenSkipSpace(pos, 0, -1, 0), -1);
    EXPECT_EQ(ast.GetCurTokenSkipSpace(pos, 1, 0, 0), -1); // start > end
}

// Test GetCurTokenByPos function
TEST(ArkASTTest, GetCurTokenByPos) {
    ArkAST ast = CreateArkAST();

    // Test empty tokens case
    Cangjie::Position pos{0, 1, 1};
    EXPECT_EQ(ast.GetCurTokenByPos(pos, 0, -1, false), -1);
    EXPECT_EQ(ast.GetCurTokenByPos(pos, 1, 0, false), -1); // start > end
}

// Test IsFilterToken function
TEST(ArkASTTest, IsFilterToken) {
    ArkAST ast = CreateArkAST();

    // Test empty tokens case
    Cangjie::Position pos{0, 1, 1};
    EXPECT_TRUE(ast.IsFilterToken(pos)); // Should return true because tokens is empty
}

// Test IsFilterTokenInHighlight function
TEST(ArkASTTest, IsFilterTokenInHighlight) {
    ArkAST ast = CreateArkAST();

    // Test empty tokens case
    Cangjie::Position pos{0, 1, 1};
    EXPECT_TRUE(ast.IsFilterTokenInHighlight(pos)); // Should return true because tokens is empty
}

// Test GetDeclByPosition function
TEST(ArkASTTest, GetDeclByPosition) {
    ArkAST ast = CreateArkAST();

    // Test invalid position case
    Cangjie::Position pos{0, 1, 1};
    EXPECT_EQ(ast.GetDeclByPosition(pos), nullptr);
}

// Test FindDeclByNode function
TEST(ArkASTTest, FindDeclByNode) {
    ArkAST ast = CreateArkAST();

    // Test empty node case
    Ptr<Node> node = nullptr;
    EXPECT_EQ(ast.FindDeclByNode(node), nullptr);
}

// Test GetDelFromType function
TEST(ArkASTTest, GetDelFromType) {
    ArkAST ast = CreateArkAST();

    // Test empty type case
    Ptr<const Cangjie::AST::Type> type = nullptr;
    EXPECT_EQ(ast.GetDelFromType(type), nullptr);
}

// Test CheckInQuote function
TEST(ArkASTTest, CheckInQuote) {
    ArkAST ast = CreateArkAST();

    // Test empty node case
    Ptr<const Node> node = nullptr;
    Cangjie::Position pos{0, 1, 1};
    EXPECT_FALSE(ast.CheckInQuote(node, pos));
}

// Test constant definitions
TEST(ArkASTTest, Constants) {
    // Test STRING_LITERAL_SIZE constant
    EXPECT_EQ(STRING_LITERAL_SIZE, 2);

    // Test CHECK_TOKEN_KIND set contains necessary token types
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::IDENTIFIER));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::STRING_LITERAL));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::MULTILINE_STRING));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::DOLLAR_IDENTIFIER));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::INIT));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::SUPER));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::THIS));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::MAIN));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::PUBLIC));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::PRIVATE));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::PROTECTED));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::OVERRIDE));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::ABSTRACT));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::OPEN));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::REDEF));
    EXPECT_TRUE(CHECK_TOKEN_KIND.count(TokenKind::SEALED));

    // Test OPERATOR_TO_OVERLOAD set contains necessary operators
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::LSQUARE));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::RSQUARE));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::NOT));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::EXP));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::MUL));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::MOD));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::DIV));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::ADD));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::SUB));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::LSHIFT));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::RSHIFT));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::LT));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::LE));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::GT));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::GE));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::EQUAL));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::NOTEQ));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::BITAND));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::BITXOR));
    EXPECT_TRUE(OPERATOR_TO_OVERLOAD.count(TokenKind::BITOR));
}

// Test GetCurToken function with multiline string token
TEST(ArkASTTest, GetCurTokenWithMultilineString) {
    ArkAST ast = CreateArkAST();

    // Create a mock multiline string token for testing
    // This would require setting up tokens in the ast
    Cangjie::Position pos{0, 1, 1};

    // Test case where tokens vector is empty
    EXPECT_EQ(ast.GetCurToken(pos, 0, -1), -1);
}

// Test FindRealDecl function with various scenarios
TEST(ArkASTTest, FindRealDecl) {
    ArkAST ast = CreateArkAST();

    // Test with empty symbols vector
    std::vector<Symbol*> emptySyms;
    std::string query = "test query";
    Cangjie::Position pos{0, 1, 1};
    std::pair<bool, bool> isMacroOrAlias = {false, false};

    auto result = ast.FindRealDecl(ast, emptySyms, query, pos, isMacroOrAlias);
    EXPECT_TRUE(result.empty());

    // Test with null symbols
    std::vector<Symbol*> nullSyms = {nullptr};
    result = ast.FindRealDecl(ast, nullSyms, query, pos, isMacroOrAlias);
    EXPECT_TRUE(result.empty());
}

// Test FindRealGenericParamDeclForExtend function
TEST(ArkASTTest, FindRealGenericParamDeclForExtend) {
    ArkAST ast = CreateArkAST();

    // Test with empty symbols vector
    std::vector<Symbol*> emptySyms;
    std::string genericParamName = "T";

    auto result = ast.FindRealGenericParamDeclForExtend(genericParamName, emptySyms);
    EXPECT_EQ(result, nullptr);

    // Test with null symbols
    std::vector<Symbol*> nullSyms = {nullptr};
    result = ast.FindRealGenericParamDeclForExtend(genericParamName, nullSyms);
    EXPECT_EQ(result, nullptr);

    // Test with non-extend declaration symbols
    std::vector<Symbol*> nonExtendSyms;
    // This would require creating mock Symbol objects with non-EXTEND_DECL nodes
    result = ast.FindRealGenericParamDeclForExtend(genericParamName, nonExtendSyms);
    EXPECT_EQ(result, nullptr);
}


// Test IsModifierBeforeDecl function (indirectly through GetDeclByPosition)
TEST(ArkASTTest, IsModifierBeforeDecl) {
    ArkAST ast = CreateArkAST();

    // This tests the modifier filtering logic in GetDeclByPosition
    // Would require setting up specific AST structures where decls[0] is a modifier
    Cangjie::Position pos{0, 1, 1};
    auto result = ast.GetDeclByPosition(pos);
    EXPECT_EQ(result, nullptr); // Should be filtered out if it's a modifier
}

// Test macro and alias handling in FindRealDecl
TEST(ArkASTTest, FindRealDeclWithMacroAndAlias) {
    ArkAST ast = CreateArkAST();

    std::vector<Symbol*> syms;
    std::string query = "test query";
    Cangjie::Position pos{0, 1, 1};

    // Test with alias enabled
    std::pair<bool, bool> aliasEnabled = {false, true};
    auto result = ast.FindRealDecl(ast, syms, query, pos, aliasEnabled);
    EXPECT_TRUE(result.empty());

    // Test with macro enabled
    std::pair<bool, bool> macroEnabled = {true, false};
    result = ast.FindRealDecl(ast, syms, query, pos, macroEnabled);
    EXPECT_TRUE(result.empty());
}

// Test CheckInQuote function with actual MacroExpandDecl
TEST(ArkASTTest, CheckInQuoteWithMacroExpandDecl) {
    ArkAST ast = CreateArkAST();

    // Test with null MacroExpandDecl
    Ptr<const MacroExpandDecl> nullMacroDecl = nullptr;
    Cangjie::Position pos{0, 1, 1};

    // This would require creating a proper MacroExpandDecl instance
    // to test the position comparison logic
    EXPECT_FALSE(ast.CheckInQuote(nullMacroDecl, pos));
}