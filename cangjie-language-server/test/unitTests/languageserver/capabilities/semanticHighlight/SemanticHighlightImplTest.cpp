// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SemanticHighlightImpl.cpp"
#include <gtest/gtest.h>

using namespace ark;
using namespace Cangjie;
using namespace Cangjie::AST;

// 001: Decl has no annotations, result should remain empty
TEST(SemanticHighlightImplTest, AddAnnoToken001) {
    auto decl = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;

    AddAnnoToken(decl, result, tokens, sm);

}

// 002: Decl has annotation but baseExpr == nullptr, should skip
TEST(SemanticHighlightImplTest, AddAnnoToken002) {
    auto decl = Ptr<Decl>(new Decl());
    decl->annotations.push_back(OwnedPtr<Annotation>(new Annotation()));
    decl->annotations[0]->baseExpr = nullptr;
    // Even if identifier.text has content, it should not be pushed
    decl->annotations[0]->identifier = *new SrcIdentifier("ignored");

    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;

    AddAnnoToken(decl, result, tokens, sm);

}

TEST(SemanticHighlightImplTest, AddAnnoToken003) {
    auto decl = Ptr<Decl>(new Decl());
    auto anno = OwnedPtr<Annotation>(new Annotation());
    // baseExpr is not null
    anno->baseExpr = OwnedPtr<Expr>(new Expr());
    anno->identifier = *new SrcIdentifier("ignored");

    decl->annotations.push_back(std::move(anno));

    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token>         tokens;
    Cangjie::SourceManager*             sm = nullptr;

    AddAnnoToken(decl, result, tokens, sm);


    // begin == identifier.Begin(), end.column = 5 + 5 == 10
    ark::Range expectedRange = {{10, 20, 5}, {10, 20, 10}};
}

// 004: Multiple annotations, mix of valid and invalid, only generate token for valid ones
TEST(SemanticHighlightImplTest, AddAnnoToken004) {
    auto decl = Ptr<Decl>(new Decl());

    // Invalid item
    auto a1 = OwnedPtr<Annotation>(new Annotation());
    a1->baseExpr = nullptr;
    a1->identifier = *new SrcIdentifier("X");
    decl->annotations.push_back(std::move(a1));

    // Valid item
    auto a2 = OwnedPtr<Annotation>(new Annotation());
    a2->baseExpr = OwnedPtr<Expr>(new Expr());
    a2->identifier = *new SrcIdentifier("X");  // length 2
    decl->annotations.push_back(std::move(a2));

    // Another valid item
    auto a3 = OwnedPtr<Annotation>(new Annotation());
    a3->baseExpr = OwnedPtr<Expr>(new Expr());
    a3->identifier = *new SrcIdentifier("X");   // length 1
    decl->annotations.push_back(std::move(a3));

    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token>         tokens;
    Cangjie::SourceManager*             sm = nullptr;

    AddAnnoToken(decl, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, GetFuncDecl001) {
    auto decl = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Token> tokens;
    SourceManager* sm = nullptr;
    GetFuncDecl(decl, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, GetFuncDecl002) {
    auto decl = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Token> tokens;
    SourceManager* sm = nullptr;
    GetFuncDecl(decl, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, GetFuncDecl003) {
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "<invalid identifier>";
    std::vector<SemanticHighlightToken> result;
    std::vector<Token> tokens;
    SourceManager* sm = nullptr;
    GetFuncDecl(decl, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, GetFuncDecl004) {
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "~init";
    std::vector<SemanticHighlightToken> result;
    std::vector<Token> tokens;
    SourceManager* sm = nullptr;
    GetFuncDecl(decl, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, GetFuncDecl005) {
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "funcName";
    std::vector<SemanticHighlightToken> result;
    std::vector<Token> tokens;
    SourceManager* sm = nullptr;
    GetFuncDecl(decl, result, tokens, sm);
    SemanticHighlightToken expected;
    expected.kind = HighlightKind::FUNCTION_H;
    expected.range.end = {10, 20, 5 + static_cast<int>(8)};
}

TEST(SemanticHighlightImplTest, GetFuncDecl006) {
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "ignoreMe";
    decl->identifierForLsp = "lspName";
    std::vector<SemanticHighlightToken> result;
    std::vector<Token> tokens;
    SourceManager* sm = nullptr;
    GetFuncDecl(decl, result, tokens, sm);
    SemanticHighlightToken expected;
    expected.kind = HighlightKind::FUNCTION_H;
    expected.range.end = {2, 3, 4 + static_cast<int>(7)};
}

TEST(SemanticHighlightImplTest, GetPrimaryDecl001) {
    auto decl = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    GetPrimaryDecl(decl, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, GetPrimaryDecl002) {
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "MyClass";
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    GetPrimaryDecl(decl, result, tokens, sm);
    SemanticHighlightToken expected;
    expected.kind = HighlightKind::CLASS_H;
    expected.range.end   = {5, 6, 7};
}

TEST(SemanticHighlightImplTest, GetCallExpr001) {
    auto expr = Ptr<CallExpr>(new CallExpr());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    GetCallExpr(expr, result, tokens, sm);
}

// 002: symbol is null
TEST(SemanticHighlightImplTest, GetCallExpr002) {
    auto expr = Ptr<CallExpr>(new CallExpr());
    expr->resolvedFunction = Ptr<FuncDecl>(new FuncDecl());
    expr->baseFunc = OwnedPtr<Expr>(new Expr());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetCallExpr(expr, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetMemberAccess001) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    GetMemberAccess(ma, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, GetMemberAccess002) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "<invalid identifier>";
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetMemberAccess003) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "f";
    ma->end = {1,2,5};
    ma->target = Ptr<Decl>(new Decl());
    ma->target->identifier = "init";
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range lr = {{1,2,5 - 1}, {1,2,5}};
}

TEST(SemanticHighlightImplTest, GetMemberAccess004) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "C";
    ma->end = {2,3,4};
    ma->target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range lr = {{2,3,4 - 1}, {2,3,4}};
}

TEST(SemanticHighlightImplTest, GetMemberAccess005) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "P";
    ma->begin = {3,4,1};
    ma->end   = {3,4,2};
    ma->target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range lr = {{3,4,1}, {3,4,2}};
}

TEST(SemanticHighlightImplTest, GetMemberAccess006) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "v";
    ma->end = {4,5,9};
    ma->target = Ptr<VarDecl>(new VarDecl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range rr = {{4,5,10}, {4,5,10 + 1}};
}

TEST(SemanticHighlightImplTest, GetMemberAccess007) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "F";
    ma->end = {5,6,8};
    ma->target = Ptr<FuncDecl>(new FuncDecl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range rr = {{5,6,9}, {5,6,9 + 1}};
}

TEST(SemanticHighlightImplTest, GetMemberAccess008)
{
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "";
    ma->end = {6, 7, 11};
    // target remains nullptr
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range rr = {{6, 7, 11}, {6, 7, 11 + 0}};
}

// 001
TEST(SemanticHighlightImplTest, GetRefExpr001) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    GetRefExpr(expr, result, tokens, sm);
}

// 002
TEST(SemanticHighlightImplTest, GetRefExpr002) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    expr->ref.target = nullptr;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
}

// 003
TEST(SemanticHighlightImplTest, GetRefExpr003) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<Decl>(new Decl());
    expr->ref.target = decl;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
    SemanticHighlightToken expected{
        HighlightKind::CLASS_H,
        {{1,1,1}, {1,1,4}}
    };
}

// 004
TEST(SemanticHighlightImplTest, GetRefExpr004) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "init";
    expr->ref.target = decl;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
    SemanticHighlightToken expected{
        HighlightKind::CLASS_H,
        {{2,2,2}, {2,2,4}}
    };
}

// 005
TEST(SemanticHighlightImplTest, GetRefExpr005) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<FuncDecl>(new FuncDecl());
    decl->identifier = "fun";
    expr->ref.target = decl;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
    SemanticHighlightToken expected{
        HighlightKind::FUNCTION_H,
        {{3,3,3}, {3,3,4}}
    };
}

// 006
TEST(SemanticHighlightImplTest, GetRefExpr006) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<PackageDecl>();
    expr->ref.target = decl;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
    SemanticHighlightToken expected{
        HighlightKind::PACKAGE_H,
        {{4,4,4}, {4,4,7}}
    };
}

// 007
TEST(SemanticHighlightImplTest, GetRefExpr007) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<VarDecl>(new VarDecl());
    expr->ref.target = decl;
    expr->isBaseFunc = true;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
    SemanticHighlightToken expected{
        HighlightKind::VARIABLE_H,
        {{5,5,5}, {5,5,6}}
    };
}

// 008
TEST(SemanticHighlightImplTest, GetRefExpr008) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<Decl>(new Decl);
    expr->ref.target = decl;
    expr->isBaseFunc = true;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
    SemanticHighlightToken expected{
        HighlightKind::FUNCTION_H,
        {{6,6,6}, {6,6,7}}
    };
}

// 009
TEST(SemanticHighlightImplTest, GetRefExpr009)
{
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<Decl>(new Decl);
    expr->ref.target = decl;
    expr->isBaseFunc = false;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager sm;
    GetRefExpr(expr, result, tokens, &sm);
    SemanticHighlightToken expected{HighlightKind::VARIABLE_H, {{7, 7, 7}, {7, 7, 8}}};
}

TEST(SemanticHighlightImplTest, GetRefType001) {
    auto rt = Ptr<RefType>(new RefType());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    GetRefType(rt, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, GetRefType002) {
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    rt->ref.target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetRefType003) {
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    rt->ref.target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetRefType004) {
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    rt->ref.target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
    SemanticHighlightToken expected{ HighlightKind::PACKAGE_H,
        {{2,2,2}, {2,2,3}} };
}

TEST(SemanticHighlightImplTest, GetRefType005) {
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    rt->ref.target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
    SemanticHighlightToken expected{ HighlightKind::CLASS_H,
        {{3,3,3}, {3,3,4}} };
}

TEST(SemanticHighlightImplTest, GetRefType006) {
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    rt->ref.target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
    SemanticHighlightToken expected{ HighlightKind::INTERFACE_H,
        {{4,4,4}, {4,4,5}} };
}

TEST(SemanticHighlightImplTest, GetRefType007) {
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    rt->ref.target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
    SemanticHighlightToken expected{ HighlightKind::RECORD_H,
        {{5,5,5}, {5,5,6}} };
}

TEST(SemanticHighlightImplTest, GetRefType008)
{
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
    SemanticHighlightToken expected{HighlightKind::VARIABLE_H, {{6, 6, 6}, {6, 6, 7}}};
}