// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// gtest coverage for the constructor/instance node expansion APIs in
// src/Cjprof.cpp (namespace Cjprof): GetConstructorNodesBySnapshotID,
// GetRootNodesBySnapshotID, ExpandConstructorNode, ExpandInstanceNode,
// ExpandDetailNode. These APIs are currently FNDA:0 in the lcov raw data
// (LLT only exercises the HTTP server path, which does not call them
// directly through this entry point).

#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "Cjprof.h"
#include "Analyzer/Types.h"

namespace {

std::string ResolveDataFile(const std::string& name) {
    const char* env = std::getenv("CJPROF_TESTDATA_DIR");
    std::string dir = (env && *env) ? env
#ifdef CJPROF_TESTDATA_DIR
        : CJPROF_TESTDATA_DIR;
#else
        : "";
#endif
    return dir + "/" + name;
}

// Parse a fresh snapshot and return its id. Each TEST uses its own parse so
// the global snapshot registry state is isolated.
uint64_t ParseForSnapshotId(const std::string& dataName) {
    std::string heap = ResolveDataFile(dataName);
    EXPECT_TRUE(std::ifstream(heap).good()) << "missing " << heap;
    std::vector<std::string> files{heap};
    EXPECT_TRUE(Cjprof::ParseHeapSnapshotFiles(files));
    // Resolve the id by file path; the global snapshot map is shared across
    // TESTs and dedups by path, so .back() ordering is unreliable.
    return Cjprof::GetSnapshotIDByFilePath(heap);
}

}  // namespace

// GetConstructorNodesBySnapshotID returns the per-class aggregation nodes;
// each has a usable id for expansion.
TEST(CjprofExpand, GetConstructorNodesThenExpand) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    auto nodes = Cjprof::GetConstructorNodesBySnapshotID(sid);
    ASSERT_FALSE(nodes.empty()) << "expected at least one constructor node";

    const auto& first = nodes.front();
    EXPECT_FALSE(first.className.empty());
    EXPECT_GT(first.childrenCount, 0u);

    auto expanded = Cjprof::ExpandConstructorNode(sid, first.id, 0, first.childrenCount);
    EXPECT_FALSE(expanded.children.empty()) << "expansion should expose child instances";
}

// GetRootNodesBySnapshotID filters by root type set. An empty set short-circuits
// to {} in the implementation, so pass real root type values to exercise the
// RawHeapSnapshotData::GetRootNodes path.
TEST(CjprofExpand, GetRootNodesBySnapshotID) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    std::set<uint8_t> rootTypes{cjprof::HeapDumpTag::ROOT_GLOBAL, cjprof::HeapDumpTag::ROOT_LOCAL, cjprof::HeapDumpTag::ROOT_UNKNOWN};
    auto roots = Cjprof::GetRootNodesBySnapshotID(sid, rootTypes);
    SUCCEED() << "GetRootNodesBySnapshotID returned " << roots.size() << " roots";
}

// ExpandInstanceNode expands a single instance under a constructor node.
// Walk constructor nodes until we find one whose expanded instance actually
// has children/retainers — that exercises the needFields (getSubRange(children)
// + ExpandDetailNode loop) and needReference branches inside the 6-arg impl.
TEST(CjprofExpand, ExpandInstanceNode) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    auto nodes = Cjprof::GetConstructorNodesBySnapshotID(sid);
    ASSERT_FALSE(nodes.empty());

    bool foundKids = false, foundRefs = false;
    for (const auto& ctor : nodes) {
        if (ctor.childrenCount == 0) continue;
        auto expanded = Cjprof::ExpandConstructorNode(sid, ctor.id, 0, ctor.childrenCount);
        for (const auto& ch : expanded.children) {
            if (ch.id == 0) continue;
            auto inst = Cjprof::ExpandInstanceNode(sid, ch.id, 0, 100);
            if (!inst.children.empty()) foundKids = true;
            if (!inst.retainerNodes.empty()) foundRefs = true;
            if (foundKids && foundRefs) break;
        }
        if (foundKids && foundRefs) break;
    }
    EXPECT_TRUE(foundKids) << "no instance exposes children (needFields branch)";
    EXPECT_TRUE(foundRefs) << "no instance exposes retainers (needReference branch)";
}

// ExpandDetailNode: the isReference=true path exercises the retainer branch.
TEST(CjprofExpand, ExpandDetailNodeByReference) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    auto nodes = Cjprof::GetConstructorNodesBySnapshotID(sid);
    ASSERT_FALSE(nodes.empty());
    for (const auto& ctor : nodes) {
        if (ctor.childrenCount == 0) continue;
        auto expanded = Cjprof::ExpandConstructorNode(sid, ctor.id, 0, ctor.childrenCount);
        for (const auto& ch : expanded.children) {
            if (ch.id == 0) continue;
            auto detail = Cjprof::ExpandDetailNode(sid, ch.id, /*isReference=*/true, 0, 100);
            if (!detail.retainerNodes.empty()) {
                SUCCEED() << "ExpandDetailNode(isReference=true) retainerCount=" << detail.retainerCount;
                return;
            }
        }
    }
    GTEST_SKIP() << "no instance with retainers";
}

// ExpandDetailNode: the isReference=false (children) path.
TEST(CjprofExpand, ExpandDetailNodeChildren) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    auto nodes = Cjprof::GetConstructorNodesBySnapshotID(sid);
    ASSERT_FALSE(nodes.empty());
    for (const auto& ctor : nodes) {
        if (ctor.childrenCount == 0) continue;
        auto expanded = Cjprof::ExpandConstructorNode(sid, ctor.id, 0, ctor.childrenCount);
        for (const auto& ch : expanded.children) {
            if (ch.id == 0) continue;
            auto detail = Cjprof::ExpandDetailNode(sid, ch.id, /*isReference=*/false, 0, 100);
            if (!detail.children.empty()) {
                SUCCEED() << "ExpandDetailNode(isReference=false) childrenCount=" << detail.childrenCount;
                return;
            }
        }
    }
    GTEST_SKIP() << "no instance with children";
}

// QuerySnapshotCountOfResults: a non-empty keyword exercises the
// IsKeywordContained match loop. Empty keyword would match everything but
// skip the substring logic; use a real substring to hit the comparison branch.
TEST(CjprofExpand, QuerySnapshotCountOfResults) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    uint32_t allCount = Cjprof::QuerySnapshotCountOfResults("", /*isIgnoreCase=*/true, sid);
    EXPECT_GT(allCount, 0u) << "empty keyword should match all nodes";
    // A keyword unlikely to match exercises the IsKeywordContained==false path.
    uint32_t noneCount = Cjprof::QuerySnapshotCountOfResults("ZZZ_NOMATCH_ZZZ", true, sid);
    EXPECT_EQ(noneCount, 0u);
}

// QuerySnapshotNodeByIndex: fetch a single page of results. A non-zero index
// is required — index==0 short-circuits to a default node. Iterate several
// indices to exercise the per-constructor curIndex loop + the found-return.
TEST(CjprofExpand, QuerySnapshotNodeByIndex) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    uint32_t allCount = Cjprof::QuerySnapshotCountOfResults("", true, sid);
    ASSERT_GT(allCount, 0u);

    for (uint32_t idx = 0; idx <= allCount; ++idx) {
        auto node = Cjprof::QuerySnapshotNodeByIndex("", true, sid, /*length=*/5, idx);
        (void)node;
    }
    // Non-matching keyword exercises the empty-result path.
    auto none = Cjprof::QuerySnapshotNodeByIndex("ZZZ_NOMATCH_ZZZ", true, sid, 10, 1);
    EXPECT_TRUE(none.className.empty());
}

// GetThreadInfos: thread/frame info from the snapshot.
TEST(CjprofExpand, GetThreadInfos) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    auto threads = Cjprof::GetThreadInfos(sid);
    // Snapshot may or may not carry thread info; the call must not crash.
    SUCCEED() << "GetThreadInfos returned " << threads.size() << " threads";
}

// GetNodeRootpaths: paths from an instance node back to GC roots. The
// internal RawHeapSnapshotData::GetNodeRootpaths needs a real instance id
// (from ExpandConstructorNode children) AND a snapshot whose reference graph
// actually yields paths — otherwise pathList stays empty and the
// sort/truncate/build loop (the bulk of the function) is skipped. We walk the
// constructor nodes until we find an instance that produces at least one path.
TEST(CjprofExpand, GetNodeRootpaths) {
    uint64_t sid = ParseForSnapshotId("heap.data");
    auto nodes = Cjprof::GetConstructorNodesBySnapshotID(sid);
    ASSERT_FALSE(nodes.empty());

    // Walk the constructor nodes to find an instance that yields >=2 root
    // paths — that exercises the sort comparator (retainedSize/size tiebreak)
    // AND, with pathNum=1, the truncate branch (pathNum < pathList.size()).
    uint64_t instId = 0;
    for (const auto& ctor : nodes) {
        if (ctor.childrenCount == 0) continue;
        auto expanded = Cjprof::ExpandConstructorNode(sid, ctor.id, 0, ctor.childrenCount);
        for (const auto& ch : expanded.children) {
            if (ch.id == 0) continue;
            auto probe = Cjprof::GetNodeRootpaths(sid, ch.id, /*pathNum=*/-1);
            if (probe.size() >= 2) { instId = ch.id; break; }
        }
        if (instId != 0) break;
    }
    if (instId == 0) GTEST_SKIP() << "no instance yields >=2 root paths in heap.data";

    // pathNum=1 truncates a >=2 path list, hitting the pathNum<num branch.
    auto paths = Cjprof::GetNodeRootpaths(sid, instId, /*pathNum=*/1);
    EXPECT_EQ(paths.size(), 1u) << "pathNum=1 should truncate to a single path";
    SUCCEED() << "GetNodeRootpaths(pathNum=1) returned " << paths.size() << " path";
}
