// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "PositionResolver.h"

#include "gtest/gtest.h"

using namespace Cangjie;
using namespace Cangjie::AST;

namespace ark {
namespace {
class FakeNode : public Node {
public:
    FakeNode() : Node(ASTKind::DECL) {}
};

Token MakeStringToken(const std::string &value, int beginColumn)
{
    Position begin{1, 1, beginColumn};
    Position end{1, 1, beginColumn + static_cast<int>(value.size()) + 2};
    return Token(TokenKind::STRING_LITERAL, value, begin, end);
}

TEST(PositionResolverTest, ConvertsPositionAtUnicodeInterpolationStartInBothDirections)
{
    auto token = MakeStringToken("${清风明月}", 10);
    std::vector<Token> tokens{token};
    FakeNode node;

    Position astPosition{1, 1, 13};
    PositionUTF8ToIDE(tokens, astPosition, node);
    EXPECT_EQ(astPosition.column, 13);

    Position idePosition{1, 1, 13};
    PositionIDEToUTF8(tokens, idePosition, node);
    EXPECT_EQ(idePosition.column, 13);
}

TEST(PositionResolverTest, ConvertsPositionInsideUnicodeInterpolationInBothDirections)
{
    auto token = MakeStringToken("${清风明月}", 10);
    std::vector<Token> tokens{token};
    FakeNode node;

    Position astPosition{1, 1, 16};
    PositionUTF8ToIDE(tokens, astPosition, node);
    EXPECT_EQ(astPosition.column, 14);

    Position idePosition{1, 1, 14};
    PositionIDEToUTF8(tokens, idePosition, node);
    EXPECT_EQ(idePosition.column, 16);
}

TEST(PositionResolverTest, ConvertsUnicodeInterpolationAfterSupplementaryCharacter)
{
    auto token = MakeStringToken("😀${清风明月}", 10);
    std::vector<Token> tokens{token};
    FakeNode node;

    Position astPosition{1, 1, 17};
    PositionUTF8ToIDE(tokens, astPosition, node);
    EXPECT_EQ(astPosition.column, 15);

    Position idePosition{1, 1, 15};
    PositionIDEToUTF8(tokens, idePosition, node);
    EXPECT_EQ(idePosition.column, 17);
}

TEST(PositionResolverTest, DoesNotApplyLiteralDelimiterOffsetToIdentifier)
{
    Position begin{1, 1, 10};
    Position end{1, 1, 22};
    Token token(TokenKind::IDENTIFIER, "清风明月", begin, end);
    std::vector<Token> tokens{token};
    FakeNode node;

    Position astPosition{1, 1, 16};
    PositionUTF8ToIDE(tokens, astPosition, node);
    EXPECT_EQ(astPosition.column, 12);

    Position idePosition{1, 1, 12};
    PositionIDEToUTF8(tokens, idePosition, node);
    EXPECT_EQ(idePosition.column, 16);
}
} // namespace
} // namespace ark
