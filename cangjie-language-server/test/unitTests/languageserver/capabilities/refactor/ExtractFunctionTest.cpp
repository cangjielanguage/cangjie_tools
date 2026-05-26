// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "capabilities/refactor/tweaks/ExtractFunction.h"
#include "gtest/gtest.h"
#include <optional>
#include <string>

using namespace ark;

TEST(ExtractFunctionTest, DeclarationBuildsModifiersGenericsParamsAndReturnBody)
{
    ExtractedFunction function;
    function.name = "extracted";
    function.isStatic = true;
    function.isConst = true;
    function.isMut = true;
    function.generics.insert("T");
    function.generics.insert("R");
    function.params.insert(ExtractedFunction::Param{"value", "T", std::nullopt, true});
    function.params.insert(ExtractedFunction::Param{"count", "Int64", std::string("countParam"), false});
    function.body = ": R {\n    return value\n}";

    std::string declaration = function.GetFunctionDeclaration();
    EXPECT_TRUE(declaration.find("static const mut func extracted<") == 0);
    EXPECT_TRUE(declaration.find("T") != std::string::npos);
    EXPECT_TRUE(declaration.find("R") != std::string::npos);
    EXPECT_TRUE(declaration.find("value: T") != std::string::npos);
    EXPECT_TRUE(declaration.find("countParam: Int64") != std::string::npos);
    EXPECT_TRUE(declaration.find(": R {\n    return value\n}") != std::string::npos);

    ExtractedFunction::Param left{"a", "Int64", std::nullopt, false};
    ExtractedFunction::Param same{"a", "Int64", std::nullopt, false};
    ExtractedFunction::Param right{"b", "Int64", std::nullopt, false};
    EXPECT_TRUE(left == same);
    EXPECT_FALSE(left != same);
    EXPECT_TRUE(left < right);
}

TEST(ExtractFunctionTest, DeclarationHandlesEmptyAndReturnValueBodies)
{
    ExtractedFunction empty;
    empty.name = "plain";
    empty.body = "{\n}";
    EXPECT_EQ(empty.GetFunctionDeclaration(), "func plain() {\n}");

    ExtractedFunction withReturn;
    withReturn.name = "value";
    withReturn.params.insert(ExtractedFunction::Param{"source", "String", std::nullopt, false});
    withReturn.returnValue = ExtractedFunction::ReturnValue{"source", true, false};
    withReturn.body = ": String {\n    return source\n}";
    EXPECT_EQ(withReturn.GetFunctionDeclaration(), "func value(source: String) : String {\n    return source\n}");
}

TEST(ExtractFunctionTest, DeclarationHandlesIndividualModifiers)
{
    ExtractedFunction staticFunction;
    staticFunction.name = "onlyStatic";
    staticFunction.isStatic = true;
    staticFunction.body = "{\n}";
    EXPECT_EQ(staticFunction.GetFunctionDeclaration(), "static func onlyStatic() {\n}");

    ExtractedFunction constFunction;
    constFunction.name = "onlyConst";
    constFunction.isConst = true;
    constFunction.body = "{\n}";
    EXPECT_EQ(constFunction.GetFunctionDeclaration(), "const func onlyConst() {\n}");

    ExtractedFunction mutFunction;
    mutFunction.name = "onlyMut";
    mutFunction.isMut = true;
    mutFunction.body = "{\n}";
    EXPECT_EQ(mutFunction.GetFunctionDeclaration(), "mut func onlyMut() {\n}");
}

TEST(ExtractFunctionTest, DeclarationUsesSortedParamsAndNewNames)
{
    ExtractedFunction function;
    function.name = "ordered";
    function.params.insert(ExtractedFunction::Param{"zeta", "String", std::nullopt, false});
    function.params.insert(ExtractedFunction::Param{"alpha", "Int64", std::string("renamedAlpha"), false});
    function.params.insert(ExtractedFunction::Param{"middle", "Bool", std::nullopt, false});
    function.body = ": Unit {\n    ()\n}";

    EXPECT_EQ(function.GetFunctionDeclaration(),
        "func ordered(renamedAlpha: Int64, middle: Bool, zeta: String) : Unit {\n    ()\n}");
}

TEST(ExtractFunctionTest, ParamComparisonIgnoresNewNameAndGenericFlag)
{
    ExtractedFunction::Param original{"value", "T", std::nullopt, false};
    ExtractedFunction::Param renamed{"value", "T", std::string("other"), true};
    ExtractedFunction::Param differentType{"value", "String", std::nullopt, false};

    EXPECT_TRUE(original == renamed);
    EXPECT_FALSE(original != renamed);
    EXPECT_FALSE(original == differentType);
    EXPECT_TRUE(differentType < original);
}

TEST(ExtractFunctionTest, DeclarationIncludesAllGenerics)
{
    ExtractedFunction function;
    function.name = "generic";
    function.generics.insert("T");
    function.generics.insert("U");
    function.body = "{\n}";

    std::string declaration = function.GetFunctionDeclaration();
    EXPECT_TRUE(declaration.find("func generic<") != std::string::npos);
    EXPECT_TRUE(declaration.find("T") != std::string::npos);
    EXPECT_TRUE(declaration.find("U") != std::string::npos);
    EXPECT_TRUE(declaration.find(">() {\n}") != std::string::npos);
}
