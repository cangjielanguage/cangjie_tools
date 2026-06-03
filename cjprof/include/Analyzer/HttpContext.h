// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJPROF_HTTP_CONTEXT_H
#define CJPROF_HTTP_CONTEXT_H

#include <memory>
#include <vector>
#include <unordered_map>
#include "Analyzer/Types.h"

namespace cjprof {

// Forward declaration
class DominanceTreeBuilder;

// Shared data context passed to HTTP handlers
struct HttpContext
{
    const std::vector<ClassInfo>* classes = nullptr;
    const std::vector<HeapObject>* objects = nullptr;
    const std::vector<GcRoot>* gcRoots = nullptr;
    const std::vector<DominanceNode>* dominanceNodes = nullptr;
    const SnapshotInfo* snapshotInfo = nullptr;
    const std::unordered_map<uint64_t, std::string>* stringTable = nullptr;

    // m_threshold01Percent is 0.001 (0.1%): retained size > heap * m_threshold01Percent passes threshold
    double m_threshold01Percent = 0.001;
    // m_cutoff05Percent is 0.005 (0.5%): child < parent * m_cutoff05Percent is cutoff
    double m_cutoff05Percent = 0.005;
    // m_maxDepthLimit is 5: max depth for sunburst/treemap
    int m_maxDepthLimit = 5;
};

} // namespace cjprof

#endif // CJPROF_HTTP_CONTEXT_H