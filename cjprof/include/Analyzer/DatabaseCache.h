#ifndef CJPROF_DATABASE_CACHE_H
#define CJPROF_DATABASE_CACHE_H

#include <string>
#include <vector>
#include "Analyzer/Types.h"

namespace cjprof {

class DatabaseCache {
public:
    // Check if cache file exists and is readable
    static bool isCacheValid(const std::string& heapFilePath);

    // Save all parsed data to .cjprof.db
    static bool save(const std::string& heapFilePath,
                     const SnapshotInfo& snapshot,
                     const std::vector<ClassInfo>& classes,
                     const std::vector<HeapObject>& objects,
                     const std::vector<GcRoot>& gcRoots,
                     const std::vector<DominanceNode>& dominanceNodes);

    // Load all data from .cjprof.db
    static bool load(const std::string& heapFilePath,
                     SnapshotInfo& snapshot,
                     std::vector<ClassInfo>& classes,
                     std::vector<HeapObject>& objects,
                     std::vector<GcRoot>& gcRoots,
                     std::vector<DominanceNode>& dominanceNodes,
                     StringTable& stringTable);
};

} // namespace cjprof

#endif // CJPROF_DATABASE_CACHE_H
