// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "capabilities/refactor/tweaks/ExtractVariable.h"
#include "ArkAST.h"
#include "gtest/gtest.h"
#include "cangjie/AST/Node.h"

using namespace ark;
using namespace Cangjie::AST;

namespace {
template <typename T> OwnedPtr<T> MakeOwner()
{
    return OwnedPtr<T>(new T());
}

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

OwnedPtr<FuncDecl> MakeFuncDecl(Position begin, Position end)
{
    auto funcDecl = MakeOwner<FuncDecl>();
    funcDecl->begin = begin;
    funcDecl->end = end;
    funcDecl->funcBody = MakeOwner<FuncBody>();
    funcDecl->funcBody->body = MakeOwner<Block>();
    funcDecl->funcBody->body->begin = begin;
    funcDecl->funcBody->body->end = end;
    return funcDecl;
}

template <typename Callback>
void RunWithSelection(OwnedPtr<File> file, ark::Range &range, Callback callback)
{
    DiagnosticEngine diagEngine;
    auto package = MakeOwner<Package>();
    file->curPackage = package.get();
    file->filePath = "test.cj";
    package->files.emplace_back(std::move(file));

    std::pair<std::string, std::string> paths = {"test.cj", ""};
    ArkAST arkAst(paths, package->files.back().get(), diagEngine, nullptr, nullptr);
    ASSERT_TRUE(SelectionTree::CreateEach(
        arkAst, "test.cj", range.start, range.end, [&](SelectionTree selectionTree) {
            Tweak::Selection sel(arkAst, range, std::move(selectionTree), {});
            callback(sel);
            return true;
        }));
}

void ExpectRange(const ark::Range &range, const Position &start, const Position &end)
{
    EXPECT_EQ(range.start, start);
    EXPECT_EQ(range.end, end);
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

TEST(ExtractVariableTest, GetInsertRangeUsesGlobalInsertPosition)
{
    ark::Range selectedRange{{1, 6, 12}, {1, 6, 13}};
    auto file = MakeOwner<File>();
    file->package = MakeOwner<PackageSpec>();
    file->package->packagePos = {1, 1, 1};
    file->decls.emplace_back(MakeFuncDecl({1, 3, 1}, {1, 4, 1}));
    auto targetFunc = MakeFuncDecl({1, 5, 1}, {1, 7, 1});
    targetFunc->funcBody->body->body.emplace_back(MakeLiteral({1, 6, 12}, {1, 6, 13}));
    file->decls.emplace_back(std::move(targetFunc));

    ark::Range insertRange;
    Ptr<Block> block = nullptr;
    RunWithSelection(std::move(file), selectedRange, [&](const Tweak::Selection &sel) {
        ExtractVariable::GetInsertRange(sel, selectedRange, true, block, insertRange);
    });

    ExpectRange(insertRange, {1, 4, 1}, {1, 4, 1});
}

TEST(ExtractVariableTest, GetInsertRangeFindsContainingSubNodeInBlock)
{
    ark::Range selectedRange{{1, 4, 10}, {1, 4, 15}};
    auto file = MakeOwner<File>();
    auto funcDecl = MakeFuncDecl({1, 2, 1}, {1, 8, 1});
    auto blockPtr = funcDecl->funcBody->body.get();
    blockPtr->body.emplace_back(MakeLiteral({1, 3, 5}, {1, 3, 12}));
    blockPtr->body.emplace_back(nullptr);
    blockPtr->body.emplace_back(MakeLiteral({1, 4, 5}, {1, 4, 20}));
    file->decls.emplace_back(std::move(funcDecl));

    ark::Range insertRange;
    Ptr<Block> block = blockPtr;
    RunWithSelection(std::move(file), selectedRange, [&](const Tweak::Selection &sel) {
        ExtractVariable::GetInsertRange(sel, selectedRange, false, block, insertRange);
    });

    ExpectRange(insertRange, {1, 4, 5}, {1, 4, 5});
}

TEST(ExtractVariableTest, GetInsertRangeKeepsEmptyRangeWhenNoScopeMatches)
{
    ark::Range selectedRange{{1, 4, 10}, {1, 4, 15}};
    auto file = MakeOwner<File>();
    auto funcDecl = MakeFuncDecl({1, 2, 1}, {1, 8, 1});
    auto blockPtr = funcDecl->funcBody->body.get();
    blockPtr->body.emplace_back(MakeLiteral({1, 5, 5}, {1, 5, 20}));
    file->decls.emplace_back(std::move(funcDecl));

    ark::Range insertRangeWithoutBlock;
    Ptr<Block> nullBlock = nullptr;
    ark::Range insertRangeWithoutMatch;
    Ptr<Block> block = blockPtr;
    RunWithSelection(std::move(file), selectedRange, [&](const Tweak::Selection &sel) {
        ExtractVariable::GetInsertRange(sel, selectedRange, false, nullBlock, insertRangeWithoutBlock);
        ExtractVariable::GetInsertRange(sel, selectedRange, false, block, insertRangeWithoutMatch);
    });

    EXPECT_TRUE(insertRangeWithoutBlock.start.IsZero());
    EXPECT_TRUE(insertRangeWithoutBlock.end.IsZero());
    EXPECT_TRUE(insertRangeWithoutMatch.start.IsZero());
    EXPECT_TRUE(insertRangeWithoutMatch.end.IsZero());
}
