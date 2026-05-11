#include "Analyzer/HttpHandlers.h"
#include "Analyzer/Types.h"
#include "Analyzer/Logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace cjprof {

// Helper to get class name from class ID
static std::string getClassName(const HttpContext& ctx, uint64_t objectId) {
    if (!ctx.objects || !ctx.classes) {
        return "unknown";
    }

    // Find the object to get its class_id
    for (const auto& obj : *ctx.objects) {
        if (obj.object_id == objectId) {
            // If class_id is 0, return category name (e.g., PRIMITIVE_ARRAY)
            if (obj.class_id == 0) {
                switch (obj.category) {
                    case ObjectCategory::PRIMITIVE_ARRAY: return "PRIMITIVE_ARRAY";
                    case ObjectCategory::OBJECT_ARRAY: return "OBJECT_ARRAY";
                    case ObjectCategory::STRUCT_ARRAY: return "STRUCT_ARRAY";
                    case ObjectCategory::PINNED_OBJECT: return "PINNED_OBJECT";
                    case ObjectCategory::LARGE_OBJECT: return "LARGE_OBJECT";
                    case ObjectCategory::UNMOVABLE_OBJECT: return "UNMOVABLE_OBJECT";
                    default: return "unknown";
                }
            }

            // Find the class - class_name is already filled by parser
            for (const auto& cls : *ctx.classes) {
                if (cls.class_id == obj.class_id) {
                    if (!cls.class_name.empty()) {
                        return cls.class_name;
                    }
                }
            }
            break;
        }
    }
    return "unknown";
}

std::string HttpHandlers::handleSnapshot(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/snapshot");

    if (!ctx.snapshotInfo) {
        return R"({"heap_total_size":0,"object_count":0,"gc_root_count":0,"used_size":0})";
    }

    std::ostringstream oss;
    oss << "{";
    oss << "\"heap_total_size\":" << ctx.snapshotInfo->heap_total_size << ",";
    oss << "\"object_count\":" << ctx.snapshotInfo->object_count << ",";
    oss << "\"gc_root_count\":" << ctx.snapshotInfo->gc_root_count << ",";
    oss << "\"used_size\":" << ctx.snapshotInfo->used_size;
    oss << "}";
    return oss.str();
}

std::string HttpHandlers::handleDominanceTree(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/dominance/tree");

    std::ostringstream oss;
    oss << "{\"nodes\":[";

    if (!ctx.dominanceNodes) {
        oss << "],\"cutoff_count\":0}";
        return oss.str();
    }

    // Section 9.2: 0.1% threshold - filter out nodes smaller than this
    // "总堆空间" refers to used heap size, not max capacity
    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 1;
    if (usedHeap == 0) usedHeap = 1;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.threshold01Percent);
    if (threshold01 == 0) threshold01 = 1; // Ensure at least 1 byte threshold

    // Build a map for quick parent lookup
    std::unordered_map<uint64_t, uint64_t> parentRetainedSize;
    for (const auto& node : *ctx.dominanceNodes) {
        parentRetainedSize[node.object_id] = node.retained_size;
    }

    bool first = true;
    int cutoffCount = 0;
    int totalSkipped = 0;

    // Track which parents have cutoff children and how many
    std::unordered_map<uint64_t, int> parentCutoffCount;

    for (const auto& node : *ctx.dominanceNodes) {
        // Section 9.2: Skip nodes that exceed max depth
        if (node.depth > ctx.maxDepthLimit) {
            totalSkipped++;
            continue;
        }

        // Section 9.2: 0.1% threshold - skip if retained_size < heap_total_size * 0.001
        if (node.retained_size < threshold01) {
            totalSkipped++;
            continue;
        }

        // Section 9.2: 0.5% cutoff rule - check if parent exists and child is < 0.5% of parent
        // If cutoff, DON'T include the child - instead track for cutoff node
        bool isCutoff = false;
        if (node.parent_id != 0) {
            auto it = parentRetainedSize.find(node.parent_id);
            if (it != parentRetainedSize.end() && it->second > 0) {
                uint64_t parentRetained = it->second;
                uint64_t cutoffThreshold = static_cast<uint64_t>(parentRetained * ctx.cutoff05Percent);
                if (node.retained_size < cutoffThreshold) {
                    isCutoff = true;
                    cutoffCount++;
                    parentCutoffCount[node.parent_id]++;
                }
            }
        }

        // Skip cutoff nodes - they are not shown, replaced by special cutoff nodes
        if (isCutoff) {
            continue;
        }

        if (!first) oss << ",";
        first = false;

        std::string className = getClassName(ctx, node.object_id);

        oss << "{";
        oss << "\"id\":" << node.object_id << ",";
        oss << "\"class_name\":\"" << className << "\",";
        oss << "\"retained_size\":" << node.retained_size << ",";
        oss << "\"shallow_size\":" << node.shallow_size << ",";
        oss << "\"depth\":" << node.depth << ",";
        oss << "\"parent_id\":" << node.parent_id << ",";
        oss << "\"instance_count\":" << node.instance_count << ",";
        oss << "\"is_clustered\":" << (node.is_class_clustered ? "true" : "false") << ",";
        oss << "\"is_cutoff\":false";
        oss << "}";
    }

    // Section 9.2: Add special cutoff nodes for parents that have filtered children
    for (const auto& entry : parentCutoffCount) {
        if (!first) oss << ",";
        first = false;

        oss << "{";
        oss << "\"id\":0,";  // Special ID=0 indicates cutoff node
        oss << "\"class_name\":\"... (" << entry.second << " children)\",";
        oss << "\"retained_size\":0,";
        oss << "\"shallow_size\":0,";
        oss << "\"depth\":0,";
        oss << "\"parent_id\":" << entry.first << ",";
        oss << "\"instance_count\":" << entry.second << ",";
        oss << "\"is_clustered\":false,";
        oss << "\"is_cutoff\":true";
        oss << "}";
    }

    oss << "],\"cutoff_count\":" << cutoffCount << ",\"total_skipped\":" << totalSkipped << "}";
    return oss.str();
}

// Section 9.3: Incremental loading - get children of a specific parent
std::string HttpHandlers::handleDominanceChildren(const HttpContext& ctx, uint64_t parentId) {
    LOG_DEBUG("Handling /api/dominance/children?parent_id={}", parentId);

    std::ostringstream oss;
    oss << "{\"nodes\":[";

    if (!ctx.dominanceNodes) {
        oss << "]}";
        return oss.str();
    }

    // For children API, we don't apply 0.1% threshold because
    // the user explicitly wants to see children of a node.
    // But we still apply 0.5% cutoff and show special cutoff node if needed.

    // Get parent's retained size for cutoff calculation
    uint64_t parentRetained = 0;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.object_id == parentId) {
            parentRetained = node.retained_size;
            break;
        }
    }

    bool first = true;
    int cutoffCount = 0;

    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id != parentId) continue;

        // Section 9.2: 0.5% cutoff check - if child is < 0.5% of parent, skip it
        bool isCutoff = false;
        if (parentRetained > 0) {
            uint64_t cutoffThreshold = static_cast<uint64_t>(parentRetained * ctx.cutoff05Percent);
            if (node.retained_size < cutoffThreshold) {
                isCutoff = true;
                cutoffCount++;
            }
        }

        // Skip cutoff children - will be replaced by special cutoff node
        if (isCutoff) {
            continue;
        }

        if (!first) oss << ",";
        first = false;

        std::string className = getClassName(ctx, node.object_id);

        oss << "{";
        oss << "\"id\":" << node.object_id << ",";
        oss << "\"class_name\":\"" << className << "\",";
        oss << "\"retained_size\":" << node.retained_size << ",";
        oss << "\"shallow_size\":" << node.shallow_size << ",";
        oss << "\"depth\":" << node.depth << ",";
        oss << "\"parent_id\":" << node.parent_id << ",";
        oss << "\"instance_count\":" << node.instance_count << ",";
        oss << "\"is_clustered\":" << (node.is_class_clustered ? "true" : "false") << ",";
        oss << "\"is_cutoff\":false";
        oss << "}";
    }

    // Add special cutoff node if any children were filtered
    if (cutoffCount > 0) {
        if (!first) oss << ",";
        first = false;

        oss << "{";
        oss << "\"id\":0,";  // Special ID=0 indicates cutoff node
        oss << "\"class_name\":\"... (" << cutoffCount << " children)\",";
        oss << "\"retained_size\":0,";
        oss << "\"shallow_size\":0,";
        oss << "\"depth\":0,";
        oss << "\"parent_id\":" << parentId << ",";
        oss << "\"instance_count\":" << cutoffCount << ",";
        oss << "\"is_clustered\":false,";
        oss << "\"is_cutoff\":true";
        oss << "}";
    }

    oss << "]}";
    return oss.str();
}

std::string HttpHandlers::handleDominanceTop10(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/dominance/top10");

    std::ostringstream oss;
    oss << "{\"items\":[";

    if (!ctx.dominanceNodes) {
        oss << "]}";
        return oss.str();
    }

    // Section 9.2: 0.1% threshold filter
    // "总堆空间" refers to used heap size, not max capacity
    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 1;
    if (usedHeap == 0) usedHeap = 1;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.threshold01Percent);
    if (threshold01 == 0) threshold01 = 1;

    // Copy filtered nodes and sort by retained_size descending
    std::vector<const DominanceNode*> sortedNodes;
    for (const auto& node : *ctx.dominanceNodes) {
        // Only include nodes above 0.1% threshold
        if (node.retained_size >= threshold01) {
            sortedNodes.push_back(&node);
        }
    }

    std::sort(sortedNodes.begin(), sortedNodes.end(), [](const DominanceNode* a, const DominanceNode* b) {
        return a->retained_size > b->retained_size;
    });

    bool first = true;
    int rank = 0;
    uint64_t totalSize = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 1;
    if (totalSize == 0) totalSize = 1;

    for (size_t i = 0; i < sortedNodes.size() && rank < 10; i++, rank++) {
        const auto* node = sortedNodes[i];

        if (!first) oss << ",";
        first = false;

        std::string className = getClassName(ctx, node->object_id);
        double percentage = (double)node->retained_size * 100.0 / (double)totalSize;

        oss << "{";
        oss << "\"rank\":" << (rank + 1) << ",";
        oss << "\"type\":\"" << className << "\",";
        oss << "\"object_id\":" << node->object_id << ",";
        oss << "\"retained_size\":" << node->retained_size << ",";
        oss << "\"percentage\":" << std::fixed << std::setprecision(2) << percentage;
        oss << "}";
    }

    oss << "]}";
    return oss.str();
}

std::string HttpHandlers::handleFragmentOverview(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/fragment/overview");

    std::ostringstream oss;
    oss << "{";

    if (ctx.snapshotInfo) {
        oss << "\"heap_limit\":" << ctx.snapshotInfo->heap_total_size << ",";
        oss << "\"used_size\":" << ctx.snapshotInfo->used_size << ",";
        double util = 0.0;
        if (ctx.snapshotInfo->heap_total_size > 0) {
            util = (double)ctx.snapshotInfo->used_size * 100.0 / (double)ctx.snapshotInfo->heap_total_size;
        }
        oss << "\"utilization\":" << std::fixed << std::setprecision(2) << util;
    } else {
        oss << "\"heap_limit\":0,\"used_size\":0,\"utilization\":0";
    }

    oss << "}";
    return oss.str();
}

std::string HttpHandlers::handleFragmentLayout(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/fragment/layout");

    // Calculate category totals and free space
    uint64_t categoryTotals[7] = {0};  // INDEX by ObjectCategory enum
    // 0=INSTANCE_OBJECT, 1=OBJECT_ARRAY, 2=STRUCT_ARRAY, 3=PRIMITIVE_ARRAY
    // 4=PINNED_OBJECT, 5=LARGE_OBJECT, 6=UNMOVABLE_OBJECT

    if (ctx.objects) {
        for (const auto& obj : *ctx.objects) {
            uint8_t catIndex = static_cast<uint8_t>(obj.category);
            if (catIndex < 7) {
                categoryTotals[catIndex] += obj.size;
            }
        }
    }

    // Calculate free space
    uint64_t heapLimit = ctx.snapshotInfo ? ctx.snapshotInfo->heap_total_size : 512 * 1024 * 1024;
    uint64_t usedSize = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 0;
    uint64_t freeSpace = (heapLimit > usedSize) ? (heapLimit - usedSize) : 0;

    // Build response
    std::ostringstream oss;
    oss << "{";

    // Categories with memory regions
    oss << "\"categories\":[";
    bool firstCat = true;

    // Map ObjectCategory to string name
    auto catName = [](ObjectCategory cat) -> const char* {
        switch (cat) {
            case ObjectCategory::INSTANCE_OBJECT: return "INSTANCE_OBJECT";
            case ObjectCategory::OBJECT_ARRAY: return "OBJECT_ARRAY";
            case ObjectCategory::STRUCT_ARRAY: return "STRUCT_ARRAY";
            case ObjectCategory::PRIMITIVE_ARRAY: return "PRIMITIVE_ARRAY";
            case ObjectCategory::PINNED_OBJECT: return "PINNED_OBJECT";
            case ObjectCategory::LARGE_OBJECT: return "LARGE_OBJECT";
            case ObjectCategory::UNMOVABLE_OBJECT: return "UNMOVABLE_OBJECT";
            default: return "UNKNOWN";
        }
    };

    // PINNED and UNMOVABLE are grouped together per requirement
    uint64_t pinnedTotal = categoryTotals[4] + categoryTotals[6];  // PINNED_OBJECT + UNMOVABLE_OBJECT
    uint64_t largeTotal = categoryTotals[5];  // LARGE_OBJECT

    // Add non-empty categories
    if (categoryTotals[0] > 0) {
        if (!firstCat) oss << ",";
        firstCat = false;
        oss << "{\"type\":\"INSTANCE_OBJECT\",\"size\":" << categoryTotals[0] << "}";
    }
    if (categoryTotals[1] > 0) {
        if (!firstCat) oss << ",";
        firstCat = false;
        oss << "{\"type\":\"OBJECT_ARRAY\",\"size\":" << categoryTotals[1] << "}";
    }
    if (categoryTotals[2] > 0) {
        if (!firstCat) oss << ",";
        firstCat = false;
        oss << "{\"type\":\"STRUCT_ARRAY\",\"size\":" << categoryTotals[2] << "}";
    }
    if (categoryTotals[3] > 0) {
        if (!firstCat) oss << ",";
        firstCat = false;
        oss << "{\"type\":\"PRIMITIVE_ARRAY\",\"size\":" << categoryTotals[3] << "}";
    }
    if (pinnedTotal > 0) {
        if (!firstCat) oss << ",";
        firstCat = false;
        oss << "{\"type\":\"PINNED_OBJECT\",\"size\":" << pinnedTotal << "}";
    }
    if (largeTotal > 0) {
        if (!firstCat) oss << ",";
        firstCat = false;
        oss << "{\"type\":\"LARGE_OBJECT\",\"size\":" << largeTotal << "}";
    }
    if (freeSpace > 0) {
        if (!firstCat) oss << ",";
        firstCat = false;
        oss << "{\"type\":\"FREE_SPACE\",\"size\":" << freeSpace << "}";
    }

    oss << "],";

    // Fragments: individual regions (simplified - one per category)
    oss << "\"fragments\":[";
    bool firstFrag = true;

    if (categoryTotals[0] > 0) {
        if (!firstFrag) oss << ",";
        firstFrag = false;
        oss << "{\"size\":" << categoryTotals[0] << ",\"type\":\"INSTANCE_OBJECT\"}";
    }
    if (categoryTotals[1] > 0) {
        if (!firstFrag) oss << ",";
        firstFrag = false;
        oss << "{\"size\":" << categoryTotals[1] << ",\"type\":\"OBJECT_ARRAY\"}";
    }
    if (categoryTotals[2] > 0) {
        if (!firstFrag) oss << ",";
        firstFrag = false;
        oss << "{\"size\":" << categoryTotals[2] << ",\"type\":\"STRUCT_ARRAY\"}";
    }
    if (categoryTotals[3] > 0) {
        if (!firstFrag) oss << ",";
        firstFrag = false;
        oss << "{\"size\":" << categoryTotals[3] << ",\"type\":\"PRIMITIVE_ARRAY\"}";
    }
    if (pinnedTotal > 0) {
        if (!firstFrag) oss << ",";
        firstFrag = false;
        oss << "{\"size\":" << pinnedTotal << ",\"type\":\"PINNED_OBJECT\"}";
    }
    if (largeTotal > 0) {
        if (!firstFrag) oss << ",";
        firstFrag = false;
        oss << "{\"size\":" << largeTotal << ",\"type\":\"LARGE_OBJECT\"}";
    }
    if (freeSpace > 0) {
        if (!firstFrag) oss << ",";
        firstFrag = false;
        oss << "{\"size\":" << freeSpace << ",\"type\":\"FREE_SPACE\"}";
    }

    oss << "]}";
    return oss.str();
}

std::string HttpHandlers::handleFragmentSummary(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/fragment/summary");

    // Calculate category totals
    uint64_t instanceTotal = 0, objectArrayTotal = 0, structArrayTotal = 0;
    uint64_t primitiveTotal = 0, pinnedTotal = 0, largeTotal = 0;

    if (ctx.objects) {
        for (const auto& obj : *ctx.objects) {
            switch (obj.category) {
                case ObjectCategory::INSTANCE_OBJECT:
                    instanceTotal += obj.size;
                    break;
                case ObjectCategory::OBJECT_ARRAY:
                    objectArrayTotal += obj.size;
                    break;
                case ObjectCategory::STRUCT_ARRAY:
                    structArrayTotal += obj.size;
                    break;
                case ObjectCategory::PRIMITIVE_ARRAY:
                    primitiveTotal += obj.size;
                    break;
                case ObjectCategory::PINNED_OBJECT:
                    pinnedTotal += obj.size;
                    break;
                case ObjectCategory::LARGE_OBJECT:
                    largeTotal += obj.size;
                    break;
                case ObjectCategory::UNMOVABLE_OBJECT:
                    // Could add separate tracking if needed
                    break;
            }
        }
    }

    std::ostringstream oss;
    oss << "{";
    oss << "\"free_total\":0,";
    oss << "\"free_max_continuous\":0,";
    oss << "\"instance_object_total\":" << instanceTotal << ",";
    oss << "\"object_array_total\":" << objectArrayTotal << ",";
    oss << "\"struct_array_total\":" << structArrayTotal << ",";
    oss << "\"primitive_array_total\":" << primitiveTotal << ",";
    oss << "\"pinned_object_total\":" << pinnedTotal << ",";
    oss << "\"large_object_total\":" << largeTotal;
    oss << ",\"top10\":[]}";
    return oss.str();
}

} // namespace cjprof
