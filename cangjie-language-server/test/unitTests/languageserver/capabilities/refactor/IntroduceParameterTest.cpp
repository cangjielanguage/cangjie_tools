// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "capabilities/refactor/tweaks/IntroduceParameter.h"
#include "gtest/gtest.h"

using namespace ark;
using namespace Cangjie::AST;

namespace {
OwnedPtr<FuncParamList> MakeParamList(Position rightParenPos)
{
    auto paramList = OwnedPtr<FuncParamList>(new FuncParamList());
    paramList->rightParenPos = rightParenPos;
    return paramList;
}

void SetFuncParamList(FuncDecl &funcDecl, OwnedPtr<FuncParamList> paramList)
{
    auto funcBody = OwnedPtr<FuncBody>(new FuncBody());
    funcBody->paramLists.emplace_back(std::move(paramList));
    funcDecl.funcBody = std::move(funcBody);
}

} // namespace
TEST(IntroduceParameterTest, InsertParameter)
{
    auto emptyParamList = MakeParamList({1, 10, 18});
    FuncDecl emptyFunc;
    SetFuncParamList(emptyFunc, std::move(emptyParamList));
    std::string paramName = "newParameter";
    std::string typeName = "Int64";
    TextEdit emptyEdit = IntroduceParameter::InsertParameter(emptyFunc, paramName, typeName);
    EXPECT_EQ(emptyEdit.newText, "newParameter: Int64");
    EXPECT_EQ(emptyEdit.range.start.line, 9);
    EXPECT_EQ(emptyEdit.range.start.column, 17);
    EXPECT_EQ(emptyEdit.range.end.line, 9);
    EXPECT_EQ(emptyEdit.range.end.column, 17);

    auto paramList = MakeParamList({1, 20, 26});
    paramList->params.emplace_back(new FuncParam());
    paramList->params.emplace_back(new FuncParam());
    FuncDecl funcWithParam;
    SetFuncParamList(funcWithParam, std::move(paramList));
    TextEdit appendEdit = IntroduceParameter::InsertParameter(funcWithParam, paramName, typeName);
    EXPECT_EQ(appendEdit.newText, ", newParameter: Int64");

    FuncDecl noBody;
    TextEdit invalidEdit = IntroduceParameter::InsertParameter(noBody, paramName, typeName);
    EXPECT_TRUE(invalidEdit.newText.empty());
}

TEST(IntroduceParameterTest, InsertParameterHandlesDifferentNamesTypesAndPositions)
{
    auto paramList = MakeParamList({2, 3, 4});
    FuncDecl funcDecl;
    SetFuncParamList(funcDecl, std::move(paramList));

    std::string unitName = "done";
    std::string unitType = "Unit";
    TextEdit unitEdit = IntroduceParameter::InsertParameter(funcDecl, unitName, unitType);
    EXPECT_EQ(unitEdit.newText, "done: Unit");
    EXPECT_EQ(unitEdit.range.start.line, 2);
    EXPECT_EQ(unitEdit.range.start.column, 3);

    auto existingParams = MakeParamList({2, 8, 12});
    auto existingParam = OwnedPtr<FuncParam>(new FuncParam());
    existingParam->identifier = "old";
    existingParams->params.emplace_back(std::move(existingParam));
    FuncDecl funcWithExistingParam;
    SetFuncParamList(funcWithExistingParam, std::move(existingParams));

    std::string stringName = "text";
    std::string stringType = "String";
    TextEdit stringEdit = IntroduceParameter::InsertParameter(funcWithExistingParam, stringName, stringType);
    EXPECT_EQ(stringEdit.newText, ", text: String");
    EXPECT_EQ(stringEdit.range.start.line, 7);
    EXPECT_EQ(stringEdit.range.start.column, 11);
}

TEST(IntroduceParameterTest, InsertParameterAppendsNamedParameterAfterPositionalParameters)
{
    auto mixedParams = MakeParamList({3, 25, 38});
    auto namedParam = OwnedPtr<FuncParam>(new FuncParam());
    namedParam->identifier = "factor";
    namedParam->isNamedParam = true;
    namedParam->begin = {3, 25, 20};
    namedParam->end = {3, 25, 35};
    mixedParams->params.emplace_back(std::move(namedParam));

    auto positionalParam = OwnedPtr<FuncParam>(new FuncParam());
    positionalParam->identifier = "base";
    positionalParam->begin = {3, 25, 37};
    positionalParam->end = {3, 25, 48};
    mixedParams->params.emplace_back(std::move(positionalParam));

    FuncDecl funcWithMixedParams;
    SetFuncParamList(funcWithMixedParams, std::move(mixedParams));
    std::string paramName = "newParam";
    std::string typeName = "Int64";

    TextEdit insertEdit = IntroduceParameter::InsertParameter(funcWithMixedParams, paramName, typeName);
    EXPECT_EQ(insertEdit.newText, ", newParam!: Int64");
    EXPECT_EQ(insertEdit.range.start.line, 24);
    EXPECT_EQ(insertEdit.range.start.column, 37);
    EXPECT_EQ(insertEdit.range.end.line, 24);
    EXPECT_EQ(insertEdit.range.end.column, 37);
}

TEST(IntroduceParameterTest, InsertParameterAppendsNamedParameterWhenAllExistingParamsAreNamed)
{
    auto namedParams = MakeParamList({4, 30, 32});
    auto namedParam = OwnedPtr<FuncParam>(new FuncParam());
    namedParam->identifier = "factor";
    namedParam->isNamedParam = true;
    namedParams->params.emplace_back(std::move(namedParam));

    FuncDecl funcWithNamedParam;
    SetFuncParamList(funcWithNamedParam, std::move(namedParams));
    std::string paramName = "newParam";
    std::string typeName = "Bool";

    TextEdit insertEdit = IntroduceParameter::InsertParameter(funcWithNamedParam, paramName, typeName);
    EXPECT_EQ(insertEdit.newText, ", newParam!: Bool");
    EXPECT_EQ(insertEdit.range.start.line, 29);
    EXPECT_EQ(insertEdit.range.start.column, 31);
}

TEST(IntroduceParameterTest, InsertParameterRejectsMissingParamList)
{
    FuncDecl missingBody;
    std::string paramName = "value";
    std::string typeName = "Float64";
    EXPECT_TRUE(IntroduceParameter::InsertParameter(missingBody, paramName, typeName).newText.empty());

    FuncDecl emptyBody;
    emptyBody.funcBody = OwnedPtr<FuncBody>(new FuncBody());
    EXPECT_TRUE(IntroduceParameter::InsertParameter(emptyBody, paramName, typeName).newText.empty());

    FuncDecl nullParamList;
    nullParamList.funcBody = OwnedPtr<FuncBody>(new FuncBody());
    nullParamList.funcBody->paramLists.emplace_back(nullptr);
    EXPECT_TRUE(IntroduceParameter::InsertParameter(nullParamList, paramName, typeName).newText.empty());
}

TEST(IntroduceParameterTest, ExtraOptionsReturnStoredOptions)
{
    IntroduceParameter introduceParameter;
    EXPECT_TRUE(introduceParameter.ExtraOptions().empty());

    introduceParameter.extraOptions["ErrorCode"] = "2";
    introduceParameter.extraOptions["suggestName"] = "extracted";

    auto options = introduceParameter.ExtraOptions();
    EXPECT_EQ(options.at("ErrorCode"), "2");
    EXPECT_EQ(options.at("suggestName"), "extracted");

    options.erase("ErrorCode");
    EXPECT_EQ(introduceParameter.ExtraOptions().count("ErrorCode"), 1);
}