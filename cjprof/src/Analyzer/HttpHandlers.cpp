#include "Analyzer/HttpHandlers.h"
#include "Analyzer/Logger.h"
#include <json/json.h>
#include <algorithm>
#include <cmath>

namespace cjprof {

// Helper to get class name from class ID
static std::string getClassName(const HttpContext& ctx, uint64_t objectId) {
    if (!ctx.objects || !ctx.classes) {
        return "unknown";
    }

    for (const auto& obj : *ctx.objects) {
        if (obj.object_id == objectId) {
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

static std::string jsonToString(const Json::Value& j) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    return Json::writeString(builder, j);
}

std::string HttpHandlers::handleSnapshot(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/snapshot");

    Json::Value j;
    if (ctx.snapshotInfo) {
        j["heap_total_size"] = static_cast<Json::UInt64>(ctx.snapshotInfo->heap_total_size);
        j["object_count"] = static_cast<Json::UInt64>(ctx.snapshotInfo->object_count);
        j["gc_root_count"] = static_cast<Json::UInt64>(ctx.snapshotInfo->gc_root_count);
        j["used_size"] = static_cast<Json::UInt64>(ctx.snapshotInfo->used_size);
    } else {
        j["heap_total_size"] = 0;
        j["object_count"] = 0;
        j["gc_root_count"] = 0;
        j["used_size"] = 0;
    }
    return jsonToString(j);
}

std::string HttpHandlers::handleDominanceTree(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/dominance/tree");

    Json::Value result;
    result["nodes"] = Json::Value(Json::arrayValue);
    result["cutoff_count"] = 0;
    result["total_skipped"] = 0;

    if (!ctx.dominanceNodes) {
        return jsonToString(result);
    }

    // Section 9.2: 0.1% threshold
    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 1;
    if (usedHeap == 0) usedHeap = 1;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.threshold01Percent);
    if (threshold01 == 0) threshold01 = 1;

    std::unordered_map<uint64_t, uint64_t> parentRetainedSize;
    for (const auto& node : *ctx.dominanceNodes) {
        parentRetainedSize[node.object_id] = node.retained_size;
    }

    int cutoffCount = 0;
    int totalSkipped = 0;
    std::unordered_map<uint64_t, int> parentCutoffCount;

    for (const auto& node : *ctx.dominanceNodes) {
        if (node.depth > ctx.maxDepthLimit) {
            totalSkipped++;
            continue;
        }

        if (node.retained_size < threshold01) {
            totalSkipped++;
            continue;
        }

        bool isCutoff = false;
        if (node.parent_id != 0) {
            auto it = parentRetainedSize.find(node.parent_id);
            if (it != parentRetainedSize.end() && it->second > 0) {
                uint64_t cutoffThreshold = static_cast<uint64_t>(it->second * ctx.cutoff05Percent);
                if (node.retained_size < cutoffThreshold) {
                    isCutoff = true;
                    cutoffCount++;
                    parentCutoffCount[node.parent_id]++;
                }
            }
        }

        if (isCutoff) {
            continue;
        }

        std::string className = getClassName(ctx, node.object_id);
        Json::Value jnode;
        jnode["id"] = static_cast<Json::UInt64>(node.object_id);
        jnode["class_name"] = className;
        jnode["retained_size"] = static_cast<Json::UInt64>(node.retained_size);
        jnode["shallow_size"] = static_cast<Json::UInt64>(node.shallow_size);
        jnode["depth"] = node.depth;
        jnode["parent_id"] = static_cast<Json::UInt64>(node.parent_id);
        jnode["instance_count"] = static_cast<Json::UInt64>(node.instance_count);
        jnode["is_clustered"] = node.is_class_clustered;
        jnode["is_cutoff"] = false;
        result["nodes"].append(jnode);
    }

    // Add special cutoff nodes
    for (const auto& entry : parentCutoffCount) {
        Json::Value jnode;
        jnode["id"] = 0;
        jnode["class_name"] = "... (" + std::to_string(entry.second) + " children)";
        jnode["retained_size"] = 0;
        jnode["shallow_size"] = 0;
        jnode["depth"] = 0;
        jnode["parent_id"] = static_cast<Json::UInt64>(entry.first);
        jnode["instance_count"] = entry.second;
        jnode["is_clustered"] = false;
        jnode["is_cutoff"] = true;
        result["nodes"].append(jnode);
    }

    result["cutoff_count"] = cutoffCount;
    result["total_skipped"] = totalSkipped;
    return jsonToString(result);
}

std::string HttpHandlers::handleDominanceChildren(const HttpContext& ctx, uint64_t parentId) {
    LOG_DEBUG("Handling /api/dominance/children?parent_id={}", parentId);

    Json::Value result;
    result["nodes"] = Json::Value(Json::arrayValue);

    if (!ctx.dominanceNodes) {
        return jsonToString(result);
    }

    uint64_t parentRetained = 0;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.object_id == parentId) {
            parentRetained = node.retained_size;
            break;
        }
    }

    int cutoffCount = 0;

    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id != parentId) continue;

        bool isCutoff = false;
        if (parentRetained > 0) {
            uint64_t cutoffThreshold = static_cast<uint64_t>(parentRetained * ctx.cutoff05Percent);
            if (node.retained_size < cutoffThreshold) {
                isCutoff = true;
                cutoffCount++;
            }
        }

        if (isCutoff) {
            continue;
        }

        std::string className = getClassName(ctx, node.object_id);
        Json::Value jnode;
        jnode["id"] = static_cast<Json::UInt64>(node.object_id);
        jnode["class_name"] = className;
        jnode["retained_size"] = static_cast<Json::UInt64>(node.retained_size);
        jnode["shallow_size"] = static_cast<Json::UInt64>(node.shallow_size);
        jnode["depth"] = node.depth;
        jnode["parent_id"] = static_cast<Json::UInt64>(node.parent_id);
        jnode["instance_count"] = static_cast<Json::UInt64>(node.instance_count);
        jnode["is_clustered"] = node.is_class_clustered;
        jnode["is_cutoff"] = false;
        result["nodes"].append(jnode);
    }

    if (cutoffCount > 0) {
        Json::Value jnode;
        jnode["id"] = 0;
        jnode["class_name"] = "... (" + std::to_string(cutoffCount) + " children)";
        jnode["retained_size"] = 0;
        jnode["shallow_size"] = 0;
        jnode["depth"] = 0;
        jnode["parent_id"] = static_cast<Json::UInt64>(parentId);
        jnode["instance_count"] = cutoffCount;
        jnode["is_clustered"] = false;
        jnode["is_cutoff"] = true;
        result["nodes"].append(jnode);
    }

    return jsonToString(result);
}

std::string HttpHandlers::handleDominanceTop10(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/dominance/top10");

    Json::Value result;
    result["items"] = Json::Value(Json::arrayValue);

    if (!ctx.dominanceNodes) {
        return jsonToString(result);
    }

    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 1;
    if (usedHeap == 0) usedHeap = 1;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.threshold01Percent);
    if (threshold01 == 0) threshold01 = 1;

    std::vector<const DominanceNode*> sortedNodes;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.retained_size >= threshold01) {
            sortedNodes.push_back(&node);
        }
    }

    std::sort(sortedNodes.begin(), sortedNodes.end(), [](const DominanceNode* a, const DominanceNode* b) {
        return a->retained_size > b->retained_size;
    });

    uint64_t totalSize = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 1;
    if (totalSize == 0) totalSize = 1;

    int rank = 0;
    for (size_t i = 0; i < sortedNodes.size() && rank < 10; i++, rank++) {
        const auto* node = sortedNodes[i];
        std::string className = getClassName(ctx, node->object_id);
        double percentage = (double)node->retained_size * 100.0 / (double)totalSize;

        Json::Value jitem;
        jitem["rank"] = rank + 1;
        jitem["type"] = className;
        jitem["object_id"] = static_cast<Json::UInt64>(node->object_id);
        jitem["retained_size"] = static_cast<Json::UInt64>(node->retained_size);
        jitem["percentage"] = std::round(percentage * 100.0) / 100.0;
        result["items"].append(jitem);
    }

    return jsonToString(result);
}

std::string HttpHandlers::handleFragmentOverview(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/fragment/overview");

    Json::Value j;
    if (ctx.snapshotInfo) {
        j["heap_limit"] = static_cast<Json::UInt64>(ctx.snapshotInfo->heap_total_size);
        j["used_size"] = static_cast<Json::UInt64>(ctx.snapshotInfo->used_size);
        double util = 0.0;
        if (ctx.snapshotInfo->heap_total_size > 0) {
            util = (double)ctx.snapshotInfo->used_size * 100.0 / (double)ctx.snapshotInfo->heap_total_size;
        }
        j["utilization"] = std::round(util * 100.0) / 100.0;
    } else {
        j["heap_limit"] = 0;
        j["used_size"] = 0;
        j["utilization"] = 0.0;
    }
    return jsonToString(j);
}

std::string HttpHandlers::handleFragmentLayout(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/fragment/layout");

    uint64_t categoryTotals[7] = {0};

    if (ctx.objects) {
        for (const auto& obj : *ctx.objects) {
            uint8_t catIndex = static_cast<uint8_t>(obj.category);
            if (catIndex < 7) {
                categoryTotals[catIndex] += obj.size;
            }
        }
    }

    uint64_t heapLimit = ctx.snapshotInfo ? ctx.snapshotInfo->heap_total_size : 512ULL * 1024 * 1024;
    uint64_t usedSize = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 0;
    uint64_t freeSpace = (heapLimit > usedSize) ? (heapLimit - usedSize) : 0;

    uint64_t pinnedTotal = categoryTotals[4] + categoryTotals[6];
    uint64_t largeTotal = categoryTotals[5];

    Json::Value result;
    result["categories"] = Json::Value(Json::arrayValue);
    result["fragments"] = Json::Value(Json::arrayValue);

    auto addCategory = [&](const char* type, uint64_t size) {
        if (size > 0) {
            Json::Value cat;
            cat["type"] = type;
            cat["size"] = static_cast<Json::UInt64>(size);
            result["categories"].append(cat);

            Json::Value frag;
            frag["size"] = static_cast<Json::UInt64>(size);
            frag["type"] = type;
            result["fragments"].append(frag);
        }
    };

    addCategory("INSTANCE_OBJECT", categoryTotals[0]);
    addCategory("OBJECT_ARRAY", categoryTotals[1]);
    addCategory("STRUCT_ARRAY", categoryTotals[2]);
    addCategory("PRIMITIVE_ARRAY", categoryTotals[3]);
    addCategory("PINNED_OBJECT", pinnedTotal);
    addCategory("LARGE_OBJECT", largeTotal);
    addCategory("FREE_SPACE", freeSpace);

    return jsonToString(result);
}

std::string HttpHandlers::handleFragmentSummary(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/fragment/summary");

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
                    break;
            }
        }
    }

    Json::Value j;
    j["free_total"] = 0;
    j["free_max_continuous"] = 0;
    j["instance_object_total"] = static_cast<Json::UInt64>(instanceTotal);
    j["object_array_total"] = static_cast<Json::UInt64>(objectArrayTotal);
    j["struct_array_total"] = static_cast<Json::UInt64>(structArrayTotal);
    j["primitive_array_total"] = static_cast<Json::UInt64>(primitiveTotal);
    j["pinned_object_total"] = static_cast<Json::UInt64>(pinnedTotal);
    j["large_object_total"] = static_cast<Json::UInt64>(largeTotal);
    j["top10"] = Json::Value(Json::arrayValue);
    return jsonToString(j);
}

} // namespace cjprof
