// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Utils.h"
#include "gtest/gtest.h"

using namespace Cangjie;
using namespace Cangjie::AST;

namespace ark {
namespace {
void CheckReferenceRangeWithoutSymbol(const std::string &name, int utf16Length)
{
    Position begin{1, 1, 9};
    Position end = begin + static_cast<int>(name.size());
    auto ref = MakeOwned<RefExpr>();
    ref->begin = begin;
    ref->end = end;
    ref->ref.identifier = SrcIdentifier(name, begin, end, false);
    std::vector<Token> tokens{Token(TokenKind::IDENTIFIER, name, begin, end)};

    auto range = GetProperRange(ref.get(), tokens);
    const int expectedStartColumn = begin.column - 1;
    EXPECT_EQ(range.start.line, 0);
    EXPECT_EQ(range.end.line, 0);
    EXPECT_EQ(range.start.column, expectedStartColumn);
    EXPECT_EQ(range.end.column, expectedStartColumn + utf16Length);
}

TEST(UnicodeReferenceRangeTest, UsesUTF16LengthForReferenceWithoutSymbol)
{
    CheckReferenceRangeWithoutSymbol("测表aa", 4);
}

TEST(UnicodeReferenceRangeTest, PreservesASCIIReferenceRangeWithoutSymbol)
{
    CheckReferenceRangeWithoutSymbol("ascii", 5);
}

TEST(UnicodeReferenceRangeTest, CountsSupplementaryCharactersAsTwoUTF16Units)
{
    CheckReferenceRangeWithoutSymbol("\xF0\xA0\x80\x80" "aa", 4);
}

TEST(UnicodeReferenceRangeTest, ConvertsStartAfterAnotherUnicodeReference)
{
    const std::string name = "测表aa";
    Position firstBegin{1, 1, 9};
    Position firstEnd = firstBegin + static_cast<int>(name.size());
    Position begin = firstEnd + 2;
    Position end = begin + static_cast<int>(name.size());
    auto ref = MakeOwned<RefExpr>();
    ref->begin = begin;
    ref->end = end;
    ref->ref.identifier = SrcIdentifier(name, begin, end, false);
    std::vector<Token> tokens{
        Token(TokenKind::IDENTIFIER, name, firstBegin, firstEnd),
        Token(TokenKind::IDENTIFIER, name, begin, end)};

    auto range = GetProperRange(ref.get(), tokens);
    EXPECT_EQ(range.start.column, 14);
    EXPECT_EQ(range.end.column, 18);
}
} // namespace
} // namespace ark
