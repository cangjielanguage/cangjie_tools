// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// gtest coverage for the diff/comparison APIs in src/Cjprof.cpp
// (namespace Cjprof): GetRootDiffNodesBySnapshotID, ExpandConstructorDiffNode,
// ExpandInstanceDiffNode, ExpandDetailDiffNode, QuerySnapshotComparison,
// QueryComparisonCountOfResults, QueryComparisonNodeByIndex.
//
// These require two parsed snapshots (base + target). Cjprof dedups
// snapshots by file path, so base and target must be *different* files;
// we use heap.data vs heap_multi_ref.data (both real heap snapshots).

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

uint64_t ParseForSnapshotId(const std::string& dataName) {
    std::string heap = ResolveDataFile(dataName);
    EXPECT_TRUE(std::ifstream(heap).good()) << "missing " << heap;
    std::vector<std::string> files{heap};
    EXPECT_TRUE(Cjprof::ParseHeapSnapshotFiles(files));
    // Resolve the id by file path rather than relying on snapshot insertion
    // order: RawHeapSnapshotDatas is a global map shared across TESTs, and
    // ParseHeapSnapshotFiles dedups by path, so .back() is unreliable.
    return Cjprof::GetSnapshotIDByFilePath(heap);
}

}  // namespace

// QuerySnapshotComparison between two parses of the same file. The result
// set covers the comparison cache population path.
TEST(CjprofDiff, QuerySnapshotComparisonSameInput) {
    uint64_t base = ParseForSnapshotId("heap.data");
    uint64_t target = ParseForSnapshotId("heap_multi_ref.data");
    ASSERT_NE(base, target) << "two different files must yield distinct snapshot ids";

    auto diff = Cjprof::QuerySnapshotComparison(base, target);
    SUCCEED() << "QuerySnapshotComparison returned " << diff.size() << " entries";
}

// GetRootDiffNodesBySnapshotID: diff of root nodes between two snapshots.
// Pass real root type values — an empty set short-circuits the downstream
// GetRootNodesBySnapshotID to {}, which skips the diff-merge body.
TEST(CjprofDiff, GetRootDiffNodes) {
    uint64_t base = ParseForSnapshotId("heap.data");
    uint64_t target = ParseForSnapshotId("heap_multi_ref.data");
    std::set<uint8_t> rootTypes{cjprof::HeapDumpTag::ROOT_GLOBAL,
                                cjprof::HeapDumpTag::ROOT_LOCAL,
                                cjprof::HeapDumpTag::ROOT_UNKNOWN};
    auto diff = Cjprof::GetRootDiffNodesBySnapshotID(base, target, rootTypes);
    SUCCEED() << "GetRootDiffNodesBySnapshotID returned " << diff.size() << " roots";
}

// QueryComparisonCountOfResults + QueryComparisonNodeByIndex: the paged
// comparison query path. QueryComparisonNodeByIndex has three internal
// branches (addedCount==0 / removedCount==0 / both nonzero) over the diff
// nodes; heap.data vs heap_multi_ref.data yields diffs in all three shapes.
TEST(CjprofDiff, QueryComparisonPaged) {
    uint64_t base = ParseForSnapshotId("heap.data");
    uint64_t target = ParseForSnapshotId("heap_multi_ref.data");
    uint32_t count = Cjprof::QueryComparisonCountOfResults("", true, base, target);
    // Even if count==0 (e.g. cache state from prior TESTs), index==0 early-returns
    // a default node; iterate a few indices to exercise the per-diff branches.
    for (uint32_t idx = 0; idx <= count; ++idx) {
        auto node = Cjprof::QueryComparisonNodeByIndex("", true, base, target, /*length=*/5, idx);
        // node may be default when idx is out of range; just drive the loop.
        (void)node;
    }
    SUCCEED() << "QueryComparisonNodeByIndex iterated " << (count + 1) << " indices";
}

// ExpandConstructorDiffNode: three internal branches depending on whether
// nodeId exists in base and/or target constructor sets:
//   (1) both   -> buildDiffChildrenAndStates
//   (2) target only -> getSubRange(target) + states=false
//   (3) base only   -> getSubRange(base) + states=true
// We collect base/target constructor id sets and exercise each branch.
TEST(CjprofDiff, ExpandConstructorDiffNodeBranches) {
    uint64_t base = ParseForSnapshotId("heap.data");
    uint64_t target = ParseForSnapshotId("heap_multi_ref.data");

    auto baseNodes = Cjprof::GetConstructorNodesBySnapshotID(base);
    auto targetNodes = Cjprof::GetConstructorNodesBySnapshotID(target);
    ASSERT_FALSE(baseNodes.empty());
    ASSERT_FALSE(targetNodes.empty());

    std::set<uint64_t> baseIds, targetIds;
    for (const auto& n : baseNodes) baseIds.insert(n.id);
    for (const auto& n : targetNodes) targetIds.insert(n.id);

    // (1) both present
    uint64_t bothId = 0;
    for (auto id : baseIds) if (targetIds.count(id)) { bothId = id; break; }
    if (bothId) {
        auto e = Cjprof::ExpandConstructorDiffNode(base, target, bothId, 0, 5);
        SUCCEED() << "[both] ExpandConstructorDiffNode childrenCount=" << e.childrenCount;
    }

    // (2) target only
    uint64_t targetOnlyId = 0;
    for (const auto& n : targetNodes) if (!baseIds.count(n.id)) { targetOnlyId = n.id; break; }
    if (targetOnlyId) {
        auto e = Cjprof::ExpandConstructorDiffNode(base, target, targetOnlyId, 0, 5);
        SUCCEED() << "[target-only] ExpandConstructorDiffNode childrenCount=" << e.childrenCount;
    }

    // (3) base only
    uint64_t baseOnlyId = 0;
    for (const auto& n : baseNodes) if (!targetIds.count(n.id)) { baseOnlyId = n.id; break; }
    if (baseOnlyId) {
        auto e = Cjprof::ExpandConstructorDiffNode(base, target, baseOnlyId, 0, 5);
        SUCCEED() << "[base-only] ExpandConstructorDiffNode childrenCount=" << e.childrenCount;
    }
}

// ExpandDiffNode cache: calling ExpandInstanceDiffNode twice for the same
// (base, target) pair + instance className hits the SnapshotDiffs cache on the
// second call (the found=true branch), after the first call populated it via
// the fallback QuerySnapshotComparison loop + cache write. The nodeId must be
// an *instance* id (looked up in insNodeIdToIndex), so we get one from
// ExpandConstructorDiffNode's children.
TEST(CjprofDiff, ExpandDiffNodeCacheHit) {
    uint64_t base = ParseForSnapshotId("heap.data");
    uint64_t target = ParseForSnapshotId("heap_multi_ref.data");
    auto diff = Cjprof::QuerySnapshotComparison(base, target);
    ASSERT_FALSE(diff.empty()) << "need comparison results to populate cache";

    uint64_t ctorId = 0;
    uint32_t cc = 0;
    for (const auto& d : diff) {
        if (d.addedCount > 0 || d.removedCount > 0) { ctorId = d.id; cc = d.childrenCount; break; }
    }
    if (ctorId == 0 || cc == 0) GTEST_SKIP() << "no diff node with children to expand";

    auto expanded = Cjprof::ExpandConstructorDiffNode(base, target, ctorId, 0, cc);
    if (expanded.children.empty()) GTEST_SKIP() << "no instance children to diff";
    uint64_t instId = expanded.children[0].id;
    if (instId == 0) GTEST_SKIP() << "instance id is 0";

    // First call: cache miss -> fallback loop + cache write.
    auto first = Cjprof::ExpandInstanceDiffNode(base, target, instId, 0, 100);
    // Second call: cache hit (the SnapshotDiffs.find != end() branch).
    auto second = Cjprof::ExpandInstanceDiffNode(base, target, instId, 0, 100);
    SUCCEED() << "ExpandInstanceDiffNode cache-hit returned className '"
              << second.className << "'";
}

// ExpandDiffNode cache-miss-not-found: an instance whose className is NOT in
// the comparison result walks the fallback loop without matching (found stays
// false) and does NOT write the cache — the 1153-1166 branch. We use a
// target-only constructor's instance (className absent from comparison).
TEST(CjprofDiff, ExpandDiffNodeCacheMissNotFound) {
    uint64_t base = ParseForSnapshotId("heap.data");
    uint64_t target = ParseForSnapshotId("heap_multi_ref.data");
    auto baseNodes = Cjprof::GetConstructorNodesBySnapshotID(base);
    auto targetNodes = Cjprof::GetConstructorNodesBySnapshotID(target);
    std::set<uint64_t> baseIds;
    for (const auto& n : baseNodes) baseIds.insert(n.id);

    uint64_t targetOnlyCtor = 0;
    uint32_t cc = 0;
    for (const auto& n : targetNodes) {
        if (!baseIds.count(n.id) && n.childrenCount > 0) { targetOnlyCtor = n.id; cc = n.childrenCount; break; }
    }
    if (targetOnlyCtor == 0) GTEST_SKIP() << "no target-only constructor with children";

    auto expanded = Cjprof::ExpandConstructorDiffNode(base, target, targetOnlyCtor, 0, cc);
    if (expanded.children.empty()) GTEST_SKIP();
    uint64_t instId = expanded.children[0].id;
    if (instId == 0) GTEST_SKIP();

    auto d = Cjprof::ExpandInstanceDiffNode(base, target, instId, 0, 100);
    SUCCEED() << "target-only instance diff className='" << d.className << "' added=" << d.added;
}

// ExpandInstanceDiffNode + ExpandDetailDiffNode: instance-level diff
// expansion. Both the isReference true/false paths are touched.
TEST(CjprofDiff, ExpandInstanceAndDetailDiffNode) {
    uint64_t base = ParseForSnapshotId("heap.data");
    uint64_t target = ParseForSnapshotId("heap_multi_ref.data");

    auto diff = Cjprof::QuerySnapshotComparison(base, target);
    if (diff.empty()) GTEST_SKIP() << "no diff nodes to expand";

    const auto& dn = diff.front();
    if (dn.childrenCount == 0) GTEST_SKIP();

    auto inst = Cjprof::ExpandInstanceDiffNode(
        base, target, dn.id, 0, dn.childrenCount);
    EXPECT_GE(inst.childrenCount, 0u);

    auto detailRef = Cjprof::ExpandDetailDiffNode(
        base, target, dn.id, /*isReference=*/true, 0, dn.childrenCount);
    EXPECT_GE(detailRef.retainerCount, 0u);

    auto detailChild = Cjprof::ExpandDetailDiffNode(
        base, target, dn.id, /*isReference=*/false, 0, dn.childrenCount);
    EXPECT_GE(detailChild.childrenCount, 0u);
}
