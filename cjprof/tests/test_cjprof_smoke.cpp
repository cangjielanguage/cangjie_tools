// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// Smoke test for the Cjprof C++ API (src/Cjprof.cpp, namespace Cjprof).
// Verifies the basic parse -> query -> expand pipeline against a real heap
// snapshot file. Intended as the first test to validate the gtest harness
// links against libcjprof.so and finds the test data.

#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "Cjprof.h"

namespace {

std::string TestDataDir() {
    const char* env = std::getenv("CJPROF_TESTDATA_DIR");
    if (env && *env) {
        return env;
    }
#ifdef CJPROF_TESTDATA_DIR
    return CJPROF_TESTDATA_DIR;
#else
    return "";
#endif
}

std::string ResolveDataFile(const std::string& name) {
    return TestDataDir() + "/" + name;
}

}  // namespace

// Parse a single heap snapshot and confirm the snapshot is registered.
TEST(CjprofSmoke, ParseSingleHeapSnapshot) {
    std::string heap = ResolveDataFile("heap.data");
    ASSERT_FALSE(heap.empty()) << "CJPROF_TESTDATA_DIR not set / not found";
    ASSERT_TRUE(std::ifstream(heap).good())
        << "missing test data: " << heap;

    std::vector<std::string> files{heap};
    ASSERT_TRUE(Cjprof::ParseHeapSnapshotFiles(files));

    auto snapshots = Cjprof::QueryAllHeapSnapshot();
    ASSERT_EQ(snapshots.size(), 1u);
    EXPECT_FALSE(snapshots[0].filePath.empty());
}

// A second parse of the same file should be idempotent at the API level
// (the snapshot set should still be queryable).
TEST(CjprofSmoke, QuerySnapshotByFilePath) {
    std::string heap = ResolveDataFile("heap.data");
    std::vector<std::string> files{heap};
    ASSERT_TRUE(Cjprof::ParseHeapSnapshotFiles(files));

    uint64_t id = Cjprof::GetSnapshotIDByFilePath(heap);
    EXPECT_NE(id, 0u) << "expected a non-zero snapshot id for " << heap;
}

// After parsing, constructor nodes should be retrievable.
TEST(CjprofSmoke, GetConstructorNodes) {
    std::string heap = ResolveDataFile("heap.data");
    std::vector<std::string> files{heap};
    ASSERT_TRUE(Cjprof::ParseHeapSnapshotFiles(files));

    uint64_t id = Cjprof::GetSnapshotIDByFilePath(heap);
    ASSERT_NE(id, 0u);

    auto nodes = Cjprof::GetConstructorNodesBySnapshotID(id);
    EXPECT_FALSE(nodes.empty()) << "expected at least one constructor node";
}

// CleanHeapSnapshotFiles should remove the snapshot from the registry.
TEST(CjprofSmoke, CleanHeapSnapshotFiles) {
    std::string heap = ResolveDataFile("heap.data");
    std::vector<std::string> files{heap};
    ASSERT_TRUE(Cjprof::ParseHeapSnapshotFiles(files));

    auto before = Cjprof::QueryAllHeapSnapshot();
    ASSERT_FALSE(before.empty());
    std::vector<uint64_t> ids{before[0].id};
    Cjprof::CleanHeapSnapshotFiles(ids);

    auto after = Cjprof::QueryAllHeapSnapshot();
    EXPECT_TRUE(after.empty()) << "snapshot should be removed after clean";
}
