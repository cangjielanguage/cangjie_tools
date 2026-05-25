// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
#include "MemIndex.h"
#include <gtest/gtest.h>

using namespace ark::lsp;

TEST(MemIndexTest, GetPackageRelation001) {
    auto rel = GetPackageRelation("com.example", "com.example.sub");
    EXPECT_EQ(rel, PackageRelation::PARENT);
}

TEST(MemIndexTest, GetPackageRelation002) {
    auto rel = GetPackageRelation("com.example.sub", "com.example");
    EXPECT_EQ(rel, PackageRelation::CHILD);
}

TEST(MemIndexTest, GetPackageRelation003) {
    auto rel = GetPackageRelation("com.a", "com.b");
    EXPECT_EQ(rel, PackageRelation::SAME_MODULE);
}

TEST(MemIndexTest, GetPackageRelation004) {
    auto rel = GetPackageRelation("org.util", "com.example");
    EXPECT_EQ(rel, PackageRelation::NONE);
}

TEST(MemIndexTest, Relations001) {
    MemIndex mem;

    // Prepare a single package with four relations:
    // rA: subject matches, predicate matches, object differs
    // rB: subject differs, predicate matches, object matches
    // rC: subject matches, predicate matches, object matches (both branches)
    // rD: subject matches, predicate differs, object matches (predicate mismatch)
    Relation rA{ SymbolID(1), RelationKind::BASE_OF,    SymbolID(2) };
    Relation rB{ SymbolID(3), RelationKind::BASE_OF,    SymbolID(1) };
    Relation rC{ SymbolID(1), RelationKind::BASE_OF,    SymbolID(1) };
    Relation rD{ SymbolID(1), RelationKind::EXTEND,     SymbolID(1) };

    mem.pkgRelationsMap["pkg1"] = RelationSlab{ rA, rB, rC, rD };

    RelationsRequest req;
    req.id = SymbolID(1);
    req.predicate = RelationKind::BASE_OF;

    std::vector<Relation> seen;
    mem.Relations(req, [&](const Relation &rel) {
        seen.push_back(rel);
    });

    // Should invoke callback exactly for:
    //  rA once (subject match)
    //  rB once (object match)
    //  rC twice (both subject and object match)
    ASSERT_EQ(seen.size(), 4u);

    // Check ordering and content
    EXPECT_EQ(seen[0].subject, rA.subject);
    EXPECT_EQ(seen[0].object,  rA.object);

    EXPECT_EQ(seen[1].subject, rB.subject);
    EXPECT_EQ(seen[1].object,  rB.object);

    EXPECT_EQ(seen[2].subject, rC.subject);
    EXPECT_EQ(seen[2].object,  rC.object);

    EXPECT_EQ(seen[3].subject, rC.subject);
    EXPECT_EQ(seen[3].object,  rC.object);
}

TEST(MemIndexTest, FindComment001) {
    MemIndex mem;
    Symbol sym;

    // Inner comment
    Token tokenInner(TokenKind::COMMENT);
    tokenInner.SetValue("inner comment");
    AST::CommentGroup innerGroup;
    innerGroup.cms.push_back(AST::Comment{
        AST::CommentStyle::OTHER,
        AST::CommentKind::BLOCK,
        tokenInner
    });

    // Leading comment
    Token tokenLead(TokenKind::COMMENT);
    tokenLead.SetValue("lead comment");
    AST::CommentGroup leadGroup;
    leadGroup.cms.push_back(AST::Comment{
        AST::CommentStyle::LEAD_LINE,
        AST::CommentKind::LINE,
        tokenLead
    });

    // Trailing comment
    Token tokenTrail(TokenKind::COMMENT);
    tokenTrail.SetValue("trail comment");
    AST::CommentGroup trailGroup;
    trailGroup.cms.push_back(AST::Comment{
        AST::CommentStyle::TRAIL_CODE,
        AST::CommentKind::DOCUMENT,
        tokenTrail
    });

    // Assign groups to sym.comments
    sym.comments.leadingComments  = { leadGroup };
    sym.comments.innerComments    = { innerGroup };
    sym.comments.trailingComments = { trailGroup };

    std::vector<std::string> comments;
    mem.FindComment(sym, comments);

    // Order: inner → lead → trail
    ASSERT_EQ(comments.size(), 3u);
    EXPECT_EQ(comments[0], "inner comment");
    EXPECT_EQ(comments[1], "lead comment");
    EXPECT_EQ(comments[2], "trail comment");
}

TEST(MemIndexTest, FindComment002)
{
    MemIndex mem;
    Symbol sym;

    // No comment groups at all
    sym.comments = AST::CommentGroups();

    std::vector<std::string> comments;
    mem.FindComment(sym, comments);

    EXPECT_TRUE(comments.empty());
}

TEST(MemIndexTest, FindCrossSymbolByName001) {
    MemIndex mem;
    std::vector<CrossSymbol> seen;

    // No entries in pkgCrossSymsMap → callback should never be invoked
    mem.FindCrossSymbolByName("pkg", "sym", false,
        [&](const CrossSymbol &cs) { seen.push_back(cs); });
    EXPECT_TRUE(seen.empty());
}

TEST(MemIndexTest, FindCrossSymbolByName002) {
    MemIndex mem;
    std::vector<CrossSymbol> seen;

    // Package present but symbol name does not match → still no callbacks
    CrossSymbol cs;
    cs.name = "other";
    mem.pkgCrossSymsMap["pkg"] = { cs };

    mem.FindCrossSymbolByName("pkg", "sym", false,
        [&](const CrossSymbol &cs) { seen.push_back(cs); });
    EXPECT_TRUE(seen.empty());
}

TEST(MemIndexTest, FindCrossSymbolByName003) {
    MemIndex mem;
    std::vector<CrossSymbol> seen;

    // Package present and symbol name matches → one callback
    CrossSymbol cs;
    cs.name = "sym";
    mem.pkgCrossSymsMap["pkg"] = { cs };

    mem.FindCrossSymbolByName("pkg", "sym", false,
        [&](const CrossSymbol &c) { seen.push_back(c); });
    ASSERT_EQ(seen.size(), 1u);
    EXPECT_EQ(seen[0].name, "sym");
}