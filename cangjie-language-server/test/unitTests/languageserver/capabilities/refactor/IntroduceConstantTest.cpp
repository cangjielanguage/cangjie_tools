// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "capabilities/refactor/tweaks/IntroduceConstant.h"
#include "gtest/gtest.h"

using namespace ark;

TEST(IntroduceConstantTest, ExtraOptionsReflectStoredOptions)
{
    IntroduceConstant introduceConstant;
    EXPECT_TRUE(introduceConstant.ExtraOptions().empty());

    introduceConstant.extraOptions["ErrorCode"] = "2";
    introduceConstant.extraOptions["suggestName"] = "constAnswer";
    EXPECT_EQ(introduceConstant.ExtraOptions().at("ErrorCode"), "2");
    EXPECT_EQ(introduceConstant.ExtraOptions().at("suggestName"), "constAnswer");
}

TEST(IntroduceConstantTest, MetadataMatchesCodeActionContract)
{
    IntroduceConstant introduceConstant;
    EXPECT_EQ(introduceConstant.Id(), "IntroduceConstant");
    EXPECT_EQ(introduceConstant.Title(), "Introduce expression to constant variable");
    EXPECT_EQ(introduceConstant.Kind(), CodeAction::REFACTOR_KIND);
}

TEST(IntroduceConstantTest, ExtraOptionsReturnsSnapshot)
{
    IntroduceConstant introduceConstant;
    introduceConstant.extraOptions["ErrorCode"] = "3";

    auto options = introduceConstant.ExtraOptions();
    options["ErrorCode"] = "changed";
    options["suggestName"] = "localOnly";

    EXPECT_EQ(introduceConstant.ExtraOptions().at("ErrorCode"), "3");
    EXPECT_EQ(introduceConstant.ExtraOptions().count("suggestName"), 0);
}
