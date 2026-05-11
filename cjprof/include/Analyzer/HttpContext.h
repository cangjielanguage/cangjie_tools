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
struct HttpContext {
    const std::vector<ClassInfo>* classes = nullptr;
    const std::vector<HeapObject>* objects = nullptr;
    const std::vector<GcRoot>* gcRoots = nullptr;
    const std::vector<DominanceNode>* dominanceNodes = nullptr;
    const SnapshotInfo* snapshotInfo = nullptr;
    const std::unordered_map<uint64_t, std::string>* stringTable = nullptr;

    // Performance optimization constants (Section 9.2)
    double threshold01Percent = 0.001;  // 0.1% - Retained Size > heap_total_size * 0.001
    double cutoff05Percent = 0.005;      // 0.5% - cutoff if child < parent * 0.005
    int maxDepthLimit = 5;               // Max depth for sunburst/treemap
};

} // namespace cjprof

#endif // CJPROF_HTTP_CONTEXT_H
