// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
#include "MemIndex.h"
#include <gtest/gtest.h>

using namespace ark::lsp;

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