// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "capabilities/refactor/tweaks/ExtractFunction.h"
#include "capabilities/refactor/tweaks/ExtractInterface.h"
#include "capabilities/refactor/tweaks/ExtractVariable.h"
#include "capabilities/refactor/tweaks/InlineFunction.h"
#include "capabilities/refactor/tweaks/InlineVariable.h"
#include "capabilities/refactor/tweaks/IntroduceConstant.h"
#include "capabilities/refactor/tweaks/IntroduceField.h"
#include "capabilities/refactor/tweaks/IntroduceParameter.h"
#include "gtest/gtest.h"

using namespace ark;

TEST(RefactorTweakMetadataTest, UsesExpectedIdsTitlesAndKind)
{
    ExtractFunction extractFunction;
    EXPECT_EQ(extractFunction.Id(), "ExtractFunction");
    EXPECT_EQ(extractFunction.Title(), "Extract to function");
    EXPECT_EQ(extractFunction.Kind(), CodeAction::REFACTOR_KIND);

    ExtractInterface extractInterface;
    EXPECT_EQ(extractInterface.Id(), "ExtractInterface");
    EXPECT_EQ(extractInterface.Title(), "Extract to interface");
    EXPECT_EQ(extractInterface.Kind(), CodeAction::REFACTOR_KIND);

    ExtractVariable extractVariable;
    EXPECT_EQ(extractVariable.Id(), "ExtractVariable");
    EXPECT_EQ(extractVariable.Title(), "Extract expression to variable");
    EXPECT_EQ(extractVariable.Kind(), CodeAction::REFACTOR_KIND);

    InlineFunction inlineFunction;
    EXPECT_EQ(inlineFunction.Id(), "InlineFunction");
    EXPECT_EQ(inlineFunction.Title(), "Inline function");
    EXPECT_EQ(inlineFunction.Kind(), CodeAction::REFACTOR_KIND);

    InlineVariable inlineVariable;
    EXPECT_EQ(inlineVariable.Id(), "InlineVariable");
    EXPECT_EQ(inlineVariable.Title(), "Inline variable");
    EXPECT_EQ(inlineVariable.Kind(), CodeAction::REFACTOR_KIND);

    IntroduceConstant introduceConstant;
    EXPECT_EQ(introduceConstant.Id(), "IntroduceConstant");
    EXPECT_EQ(introduceConstant.Title(), "Introduce expression to constant variable");
    EXPECT_EQ(introduceConstant.Kind(), CodeAction::REFACTOR_KIND);

    IntroduceField introduceField;
    EXPECT_EQ(introduceField.Id(), "IntroduceField");
    EXPECT_EQ(introduceField.Title(), "Introduce expression to field");
    EXPECT_EQ(introduceField.Kind(), CodeAction::REFACTOR_KIND);

    IntroduceParameter introduceParameter;
    EXPECT_EQ(introduceParameter.Id(), "IntroduceParameter");
    EXPECT_EQ(introduceParameter.Title(), "Introduce expression to parameter");
    EXPECT_EQ(introduceParameter.Kind(), CodeAction::REFACTOR_KIND);
}

TEST(RefactorTweakMetadataTest, ExtraOptionsReturnStoredOptions)
{
    ExtractVariable extractVariable;
    extractVariable.extraOptions["ErrorCode"] = "2";
    EXPECT_EQ(extractVariable.ExtraOptions().at("ErrorCode"), "2");

    IntroduceConstant introduceConstant;
    introduceConstant.extraOptions["suggestName"] = "answer";
    EXPECT_EQ(introduceConstant.ExtraOptions().at("suggestName"), "answer");

    IntroduceField introduceField;
    introduceField.extraOptions["visibility"] = "private";
    EXPECT_EQ(introduceField.ExtraOptions().at("visibility"), "private");

    IntroduceParameter introduceParameter;
    introduceParameter.extraOptions["typeName"] = "Int64";
    EXPECT_EQ(introduceParameter.ExtraOptions().at("typeName"), "Int64");
}
