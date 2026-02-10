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

// --- AddAnnoToken Tests ---

TEST(SemanticHighlightImplTest, AddAnnoToken001) {
    auto decl = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    AddAnnoToken(decl, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, AddAnnoToken002) {
    auto decl = Ptr<Decl>(new Decl());
    decl->annotations.push_back(OwnedPtr<Annotation>(new Annotation()));
    decl->annotations[0]->baseExpr = nullptr;
    decl->annotations[0]->identifier = *new SrcIdentifier("ignored");

    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    AddAnnoToken(decl, result, tokens, sm);
}

TEST(SemanticHighlightImplTest, AddAnnoToken003) {
    auto decl = Ptr<Decl>(new Decl());
    auto anno = OwnedPtr<Annotation>(new Annotation());
    anno->baseExpr = OwnedPtr<Expr>(new Expr());
    anno->identifier = *new SrcIdentifier("ignored");
    decl->annotations.push_back(std::move(anno));

    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    AddAnnoToken(decl, result, tokens, sm);

    ark::Range expectedRange = {{10, 20, 5}, {10, 20, 10}};
}

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
    a2->identifier = *new SrcIdentifier("X");
    decl->annotations.push_back(std::move(a2));

    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sm = nullptr;
    AddAnnoToken(decl, result, tokens, sm);
}

// --- GetFuncDecl Tests ---

TEST(SemanticHighlightImplTest, GetFuncDecl001) {
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
    GetFuncDecl(decl, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetFuncDecl005) {
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "funcName";
    std::vector<SemanticHighlightToken> result;
    std::vector<Token> tokens;
    GetFuncDecl(decl, result, tokens, nullptr);
    SemanticHighlightToken expected;
    expected.kind = HighlightKind::FUNCTION_H;
    expected.range.end = {10, 20, 13};
}

TEST(SemanticHighlightImplTest, GetFuncDecl006) {
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "ignoreMe";
    decl->identifierForLsp = "lspName";
    std::vector<SemanticHighlightToken> result;
    std::vector<Token> tokens;
    GetFuncDecl(decl, result, tokens, nullptr);
    SemanticHighlightToken expected;
    expected.kind = HighlightKind::FUNCTION_H;
    expected.range.end = {2, 3, 11};
}

// --- GetPrimaryDecl Tests ---

TEST(SemanticHighlightImplTest, GetPrimaryDecl001) {
    auto decl = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetPrimaryDecl(decl, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetPrimaryDecl002) {
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "MyClass";
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetPrimaryDecl(decl, result, tokens, nullptr);
    SemanticHighlightToken expected;
    expected.kind = HighlightKind::CLASS_H;
    expected.range.end = {5, 6, 7};
}

// --- GetCallExpr Tests ---

TEST(SemanticHighlightImplTest, GetCallExpr001) {
    auto expr = Ptr<CallExpr>(new CallExpr());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetCallExpr(expr, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetCallExpr002) {
    auto expr = Ptr<CallExpr>(new CallExpr());
    expr->resolvedFunction = Ptr<FuncDecl>(new FuncDecl());
    expr->baseFunc = OwnedPtr<Expr>(new Expr());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetCallExpr(expr, result, tokens, nullptr);
}

// --- GetMemberAccess Tests ---

TEST(SemanticHighlightImplTest, GetMemberAccess003) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "f";
    ma->end = {1, 2, 5};
    ma->target = Ptr<Decl>(new Decl());
    ma->target->identifier = "init";
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range lr = {{1, 2, 4}, {1, 2, 5}};
}

TEST(SemanticHighlightImplTest, GetMemberAccess006) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "v";
    ma->end = {4, 5, 9};
    ma->target = Ptr<VarDecl>(new VarDecl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range rr = {{4, 5, 10}, {4, 5, 11}};
}

TEST(SemanticHighlightImplTest, GetMemberAccess008) {
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "";
    ma->end = {6, 7, 11};
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetMemberAccess(ma, result, tokens, nullptr);
    ark::Range rr = {{6, 7, 11}, {6, 7, 11}};
}

// --- GetRefExpr Tests ---

TEST(SemanticHighlightImplTest, GetRefExpr002) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    expr->ref.target = nullptr;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetRefExpr003) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<Decl>(new Decl());
    expr->ref.target = decl;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
    SemanticHighlightToken expected{HighlightKind::CLASS_H, {{1, 1, 1}, {1, 1, 4}}};
}

TEST(SemanticHighlightImplTest, GetRefExpr007) {
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto decl = Ptr<VarDecl>(new VarDecl());
    expr->ref.target = decl;
    expr->isBaseFunc = true;
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefExpr(expr, result, tokens, nullptr);
    SemanticHighlightToken expected{HighlightKind::VARIABLE_H, {{5, 5, 5}, {5, 5, 6}}};
}

TEST(SemanticHighlightImplTest, GetRefExpr009) {
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

// --- GetRefType Tests ---

TEST(SemanticHighlightImplTest, GetRefType002) {
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    rt->ref.target = Ptr<Decl>(new Decl());
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
}

TEST(SemanticHighlightImplTest, GetRefType008) {
    auto rt = Ptr<RefType>(new RefType());
    rt->ref.identifier = *new SrcIdentifier("V");
    std::vector<SemanticHighlightToken> result;
    std::vector<Cangjie::Token> tokens;
    GetRefType(rt, result, tokens, nullptr);
    SemanticHighlightToken expected{HighlightKind::VARIABLE_H, {{6, 6, 6}, {6, 6, 7}}};
}