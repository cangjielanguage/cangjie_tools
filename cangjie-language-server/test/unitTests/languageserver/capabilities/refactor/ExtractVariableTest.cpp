// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "capabilities/refactor/tweaks/ExtractVariable.h"
#include "gtest/gtest.h"
#include "cangjie/AST/Node.h"

using namespace ark;
using namespace Cangjie::AST;

namespace {
OwnedPtr<LitConstExpr> MakeLiteral(Position begin, Position end)
{
    auto expr = OwnedPtr<LitConstExpr>(new LitConstExpr());
    expr->begin = begin;
    expr->end = end;
    return expr;
}

OwnedPtr<RefExpr> MakeRefToDecl(Decl &decl, Position begin, Position end)
{
    auto expr = OwnedPtr<RefExpr>(new RefExpr());
    expr->begin = begin;
    expr->end = end;
    expr->ref.target = &decl;
    return expr;
}
} // namespace

TEST(ExtractVariableTest, DealIfExprRecognizesConditionAndElseIf)
{
    IfExpr ifExpr;
    ifExpr.begin = {1, 1, 1};
    ifExpr.end = {1, 5, 1};
    ifExpr.condExpr = MakeLiteral({1, 1, 4}, {1, 1, 12});

    ark::Range condRange{{1, 1, 5}, {1, 1, 10}};
    EXPECT_TRUE(ExtractVariable::DealIfExpr(ifExpr, condRange));

    ark::Range bodyRange{{1, 2, 5}, {1, 2, 10}};
    EXPECT_FALSE(ExtractVariable::DealIfExpr(ifExpr, bodyRange));

    auto elseIf = OwnedPtr<IfExpr>(new IfExpr());
    elseIf->begin = {1, 3, 1};
    elseIf->end = {1, 5, 1};
    elseIf->condExpr = MakeLiteral({1, 3, 8}, {1, 3, 16});
    ifExpr.elseBody = std::move(elseIf);

    ark::Range elseIfCondRange{{1, 3, 9}, {1, 3, 15}};
    EXPECT_TRUE(ExtractVariable::DealIfExpr(ifExpr, elseIfCondRange));
}

TEST(ExtractVariableTest, DealIfExprRejectsMissingAndOutOfRangeConditions)
{
    IfExpr ifExpr;
    ark::Range range{{1, 1, 1}, {1, 1, 2}};
    EXPECT_FALSE(ExtractVariable::DealIfExpr(ifExpr, range));

    ifExpr.condExpr = MakeLiteral({1, 2, 1}, {1, 2, 10});
    ark::Range beforeCond{{1, 1, 1}, {1, 1, 3}};
    EXPECT_FALSE(ExtractVariable::DealIfExpr(ifExpr, beforeCond));

    auto nonIfElse = MakeLiteral({1, 4, 1}, {1, 4, 5});
    ifExpr.elseBody = std::move(nonIfElse);
    ark::Range elseRange{{1, 4, 2}, {1, 4, 4}};
    EXPECT_FALSE(ExtractVariable::DealIfExpr(ifExpr, elseRange));
}

TEST(ExtractVariableTest, DealIfExprRejectsElseIfRangeOutsideNestedCondition)
{
    IfExpr ifExpr;
    ifExpr.condExpr = MakeLiteral({1, 1, 4}, {1, 1, 12});

    auto elseIf = OwnedPtr<IfExpr>(new IfExpr());
    elseIf->begin = {1, 3, 1};
    elseIf->end = {1, 6, 1};
    elseIf->condExpr = MakeLiteral({1, 3, 8}, {1, 3, 16});
    ifExpr.elseBody = std::move(elseIf);

    ark::Range nestedBodyRange{{1, 4, 5}, {1, 4, 10}};
    EXPECT_FALSE(ExtractVariable::DealIfExpr(ifExpr, nestedBodyRange));
}

TEST(ExtractVariableTest, MatchModifierKeepsStaticModifierFromVarDecl)
{
    VarDecl varDecl;
    std::string modifier;
    ExtractVariable::MatchModifier(varDecl, modifier);
    EXPECT_TRUE(modifier.empty());

    varDecl.EnableAttr(Attribute::STATIC);
    ExtractVariable::MatchModifier(varDecl, modifier);
    EXPECT_EQ(modifier, "static ");
}

TEST(ExtractVariableTest, MatchModifierKeepsConstAndStaticConstFromVarDecl)
{
    VarDecl constDecl;
    constDecl.isConst = true;
    std::string constModifier;
    ExtractVariable::MatchModifier(constDecl, constModifier);
    EXPECT_EQ(constModifier, "const ");

    VarDecl staticConstDecl;
    staticConstDecl.EnableAttr(Attribute::STATIC);
    staticConstDecl.isConst = true;
    std::string staticConstModifier;
    ExtractVariable::MatchModifier(staticConstDecl, staticConstModifier);
    EXPECT_EQ(staticConstModifier, "static const ");
}

TEST(ExtractVariableTest, MatchModifierHandlesAssignExprTargets)
{
    VarDecl target;
    target.EnableAttr(Attribute::STATIC);
    target.isConst = true;

    AssignExpr assignExpr;
    assignExpr.leftValue = MakeRefToDecl(target, {1, 1, 1}, {1, 1, 7});

    std::string modifier;
    ExtractVariable::MatchModifier(assignExpr, modifier);
    EXPECT_EQ(modifier, "static const ");

    AssignExpr noLeftValue;
    modifier = "keep";
    ExtractVariable::MatchModifier(noLeftValue, modifier);
    EXPECT_EQ(modifier, "keep");
}

TEST(ExtractVariableTest, MatchModifierIgnoresUnrelatedNode)
{
    LitConstExpr expr;
    std::string modifier = "existing";
    ExtractVariable::MatchModifier(expr, modifier);
    EXPECT_EQ(modifier, "existing");
}
