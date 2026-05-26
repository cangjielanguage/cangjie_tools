// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Regexp.cpp"
#include <gtest/gtest.h>

TEST(RegexpTest, regexpTest001)
{
    std::optional<std::string_view> regex = "abc";
    std::optional<std::string_view> text = "abcdef";
    auto result = sqldb::regexp(regex, text);
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST(RegexpTest, regexpTest002)
{
    std::optional<std::string_view> regex = "abc";
    std::optional<std::string_view> text = "def";
    auto result = sqldb::regexp(regex, text);
    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST(RegexpTest, regexpTest003)
{
    std::optional<std::string_view> regex;
    std::optional<std::string_view> text;
    auto result = sqldb::regexp(regex, text);
    EXPECT_EQ(1, 1);
}