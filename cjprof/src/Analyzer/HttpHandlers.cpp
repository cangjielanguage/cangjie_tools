#include "Analyzer/HttpHandlers.h"
#include "Analyzer/Types.h"
#include "Analyzer/Logger.h"
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

namespace cjprof {

static std::string getClassName(const HttpContext& ctx, uint64_t objectId) {
    if (!ctx.objects) {
        return "unknown";
    }

    for (const auto& obj : *ctx.objects) {
        if (obj.object_id == objectId) {
            if (!obj.name.empty()) {
                return obj.name;
            }
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

/**
 * Cluster result containing the node, its class name, and original instance IDs.
 */
struct ClusterResult {
    DominanceNode node;
    std::string class_name;
    std::vector<uint64_t> instance_ids;  // Original object IDs for clustered nodes
};

/**
 * Cluster children by class name.
 * Multiple instances of the same class under the same parent are merged into one cluster node.
 * Returns clustered nodes sorted by total retained_size descending.
 */
static std::vector<ClusterResult> clusterByClassName(
    const HttpContext& ctx,
    const std::vector<const DominanceNode*>& children,
    uint64_t parentId)
{
    // Group children by class name
    std::unordered_map<std::string, std::vector<const DominanceNode*>> classGroups;
    for (const auto* node : children) {
        std::string className = getClassName(ctx, node->object_id);
        classGroups[className].push_back(node);
    }

    // Build clustered nodes
    std::vector<ClusterResult> clusteredNodes;
    for (auto& [className, nodes] : classGroups) {
        DominanceNode clusterNode;
        clusterNode.is_class_clustered = (nodes.size() > 1);  // Only mark as clustered if multiple instances
        clusterNode.instance_count = static_cast<uint32_t>(nodes.size());
        clusterNode.parent_id = parentId;
        clusterNode.depth = nodes.front()->depth;

        // For clustered nodes, use hash of class name as object_id
        // For single nodes, keep original object_id
        if (nodes.size() > 1) {
            clusterNode.object_id = std::hash<std::string>{}(className + std::to_string(parentId));
            // Sum sizes for clustered nodes
            clusterNode.retained_size = 0;
            clusterNode.shallow_size = 0;
            for (const auto* n : nodes) {
                clusterNode.retained_size += n->retained_size;
                clusterNode.shallow_size += n->shallow_size;
            }
        } else {
            clusterNode.object_id = nodes.front()->object_id;
            clusterNode.retained_size = nodes.front()->retained_size;
            clusterNode.shallow_size = nodes.front()->shallow_size;
        }

        ClusterResult result;
        result.node = clusterNode;
        result.class_name = className;
        // Store original instance IDs
        for (const auto* n : nodes) {
            result.instance_ids.push_back(n->object_id);
        }
        clusteredNodes.push_back(result);
    }

    // Sort by retained_size descending
    std::sort(clusteredNodes.begin(), clusteredNodes.end(), [](const ClusterResult& a, const ClusterResult& b) {
        return a.node.retained_size > b.node.retained_size;
    });

    return clusteredNodes;
}

std::string HttpHandlers::handleSnapshot(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/snapshot");

    json j;
    if (ctx.snapshotInfo) {
        j["heap_total_size"] = ctx.snapshotInfo->heap_total_size;
        j["object_count"] = ctx.snapshotInfo->object_count;
        j["gc_root_count"] = ctx.snapshotInfo->gc_root_count;
        j["used_size"] = ctx.snapshotInfo->used_size;
    } else {
        j = {{"heap_total_size", 0}, {"object_count", 0}, {"gc_root_count", 0}, {"used_size", 0}};
    }
    return j.dump();
}

std::string HttpHandlers::handleDominanceTree(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/dominance/tree");

    json result;
    result["nodes"] = json::array();
    result["cutoff_count"] = 0;
    result["total_skipped"] = 0;

    if (!ctx.dominanceNodes) {
        return result.dump();
    }

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

    // Collect root nodes (parent_id == 0 or depth == 0)
    std::vector<const DominanceNode*> rootNodes;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id == 0 || node.depth == 0) {
            if (node.depth > ctx.maxDepthLimit || node.retained_size < threshold01) {
                totalSkipped++;
                continue;
            }
            rootNodes.push_back(&node);
        }
    }

    // Cluster root nodes by class name
    auto clusteredRoots = clusterByClassName(ctx, rootNodes, 0);

    // Add clustered root nodes
    for (const auto& cluster : clusteredRoots) {
        std::string className = cluster.class_name;
        if (cluster.node.is_class_clustered) {
            className = className + " (" + std::to_string(cluster.node.instance_count) + " instances)";
        }
        json nodeJson = {
            {"id", cluster.node.object_id},
            {"class_name", className},
            {"retained_size", cluster.node.retained_size},
            {"shallow_size", cluster.node.shallow_size},
            {"depth", 0},
            {"parent_id", 0},
            {"instance_count", cluster.node.instance_count},
            {"is_clustered", cluster.node.is_class_clustered},
            {"is_cutoff", false}
        };
        // Add instance_ids for clustered nodes
        if (cluster.node.is_class_clustered && !cluster.instance_ids.empty()) {
            nodeJson["instance_ids"] = cluster.instance_ids;
        }
        result["nodes"].push_back(nodeJson);
    }

    // Group non-root nodes by parent_id for clustering
    std::unordered_map<uint64_t, std::vector<const DominanceNode*>> childrenByParent;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id == 0 || node.depth == 0) {
            continue;  // Already handled as root nodes
        }
        if (node.depth > ctx.maxDepthLimit || node.retained_size < threshold01) {
            totalSkipped++;
            continue;
        }
        childrenByParent[node.parent_id].push_back(&node);
    }

    // Cluster and add non-root nodes for each parent
    for (const auto& [parentId, children] : childrenByParent) {
        // Get parent retained size for cutoff calculation
        uint64_t parentRetained = 0;
        auto it = parentRetainedSize.find(parentId);
        if (it != parentRetainedSize.end()) {
            parentRetained = it->second;
        }

        // Cluster children by class name
        auto clusteredChildren = clusterByClassName(ctx, children, parentId);

        // Add clustered children with cutoff filtering
        for (const auto& cluster : clusteredChildren) {
            // Check cutoff
            bool isCutoff = false;
            if (parentRetained > 0) {
                uint64_t cutoffThreshold = static_cast<uint64_t>(parentRetained * ctx.cutoff05Percent);
                if (cluster.node.retained_size < cutoffThreshold) {
                    isCutoff = true;
                    cutoffCount++;
                    parentCutoffCount[parentId]++;
                }
            }

            if (isCutoff) {
                continue;
            }

            std::string className = cluster.class_name;
            if (cluster.node.is_class_clustered) {
                className = className + " (" + std::to_string(cluster.node.instance_count) + " instances)";
            }
            json nodeJson = {
                {"id", cluster.node.object_id},
                {"class_name", className},
                {"retained_size", cluster.node.retained_size},
                {"shallow_size", cluster.node.shallow_size},
                {"depth", cluster.node.depth},
                {"parent_id", parentId},
                {"instance_count", cluster.node.instance_count},
                {"is_clustered", cluster.node.is_class_clustered},
                {"is_cutoff", false}
            };
            // Add instance_ids for clustered nodes
            if (cluster.node.is_class_clustered && !cluster.instance_ids.empty()) {
                nodeJson["instance_ids"] = cluster.instance_ids;
            }
            result["nodes"].push_back(nodeJson);
        }
    }

    for (const auto& entry : parentCutoffCount) {
        result["nodes"].push_back({
            {"id", 0},
            {"class_name", "... (" + std::to_string(entry.second) + " children)"},
            {"retained_size", 0},
            {"shallow_size", 0},
            {"depth", 0},
            {"parent_id", entry.first},
            {"instance_count", entry.second},
            {"is_clustered", false},
            {"is_cutoff", true}
        });
    }

    result["cutoff_count"] = cutoffCount;
    result["total_skipped"] = totalSkipped;
    return result.dump();
}

std::string HttpHandlers::handleDominanceChildren(const HttpContext& ctx, uint64_t parentId) {
    LOG_DEBUG("Handling /api/dominance/children?parent_id={}", parentId);

    json result;
    result["nodes"] = json::array();

    if (!ctx.dominanceNodes) {
        return result.dump();
    }

    // Find parent's retained size
    uint64_t parentRetained = 0;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.object_id == parentId) {
            parentRetained = node.retained_size;
            break;
        }
    }

    // Collect and sort children
    std::vector<const DominanceNode*> children;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id == parentId) {
            children.push_back(&node);
        }
    }

    // Cluster children by class name
    auto clusteredNodes = clusterByClassName(ctx, children, parentId);

    // Add clustered children to result
    int cutoffCount = 0;
    for (const auto& cluster : clusteredNodes) {
        std::string className = cluster.class_name;
        // For clustered nodes, modify class_name to show count
        if (cluster.node.is_class_clustered) {
            className = className + " (" + std::to_string(cluster.node.instance_count) + " instances)";
        }
        json nodeJson = {
            {"id", cluster.node.object_id},
            {"class_name", className},
            {"retained_size", cluster.node.retained_size},
            {"shallow_size", cluster.node.shallow_size},
            {"depth", cluster.node.depth},
            {"parent_id", cluster.node.parent_id},
            {"instance_count", cluster.node.instance_count},
            {"is_clustered", cluster.node.is_class_clustered},
            {"is_cutoff", false}
        };
        // Add instance_ids for clustered nodes
        if (cluster.node.is_class_clustered && !cluster.instance_ids.empty()) {
            nodeJson["instance_ids"] = cluster.instance_ids;
        }
        result["nodes"].push_back(nodeJson);
    }

    return result.dump();
}

std::string HttpHandlers::handleDominanceClusterExpand(const HttpContext& ctx, const std::vector<uint64_t>& instanceIds) {
    LOG_DEBUG("Handling /api/dominance/cluster-expand with {} instance IDs", instanceIds.size());

    json result;
    result["nodes"] = json::array();

    if (!ctx.dominanceNodes) {
        return result.dump();
    }

    // Find each instance by its object_id and return as individual nodes
    for (const auto& instanceId : instanceIds) {
        for (const auto& node : *ctx.dominanceNodes) {
            if (node.object_id == instanceId) {
                std::string className = getClassName(ctx, node.object_id);
                // Check if this instance has children
                bool hasChildren = false;
                for (const auto& child : *ctx.dominanceNodes) {
                    if (child.parent_id == instanceId) {
                        hasChildren = true;
                        break;
                    }
                }
                json nodeJson = {
                    {"id", node.object_id},
                    {"class_name", className},
                    {"retained_size", node.retained_size},
                    {"shallow_size", node.shallow_size},
                    {"depth", node.depth},
                    {"parent_id", node.parent_id},
                    {"instance_count", 1},
                    {"is_clustered", false},
                    {"is_cutoff", false},
                    {"has_children", hasChildren}
                };
                result["nodes"].push_back(nodeJson);
                break;
            }
        }
    }

    return result.dump();
}

std::string HttpHandlers::handleDominanceTop10(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/dominance/top10");

    json result;
    result["items"] = json::array();

    if (!ctx.dominanceNodes) {
        return result.dump();
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

        result["items"].push_back({
            {"rank", rank + 1},
            {"type", className},
            {"object_id", node->object_id},
            {"retained_size", node->retained_size},
            {"percentage", std::round(percentage * 100.0) / 100.0}
        });
    }

    return result.dump();
}

std::string HttpHandlers::handleFragmentOverview(const HttpContext& ctx) {
    LOG_DEBUG("Handling /api/fragment/overview");

    json j;
    if (ctx.snapshotInfo) {
        j["heap_limit"] = ctx.snapshotInfo->heap_total_size;
        j["used_size"] = ctx.snapshotInfo->used_size;
        double util = 0.0;
        if (ctx.snapshotInfo->heap_total_size > 0) {
            util = (double)ctx.snapshotInfo->used_size * 100.0 / (double)ctx.snapshotInfo->heap_total_size;
        }
        j["utilization"] = std::round(util * 100.0) / 100.0;
    } else {
        j = {{"heap_limit", 0}, {"used_size", 0}, {"utilization", 0.0}};
    }
    return j.dump();
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

    json result;
    result["categories"] = json::array();
    result["fragments"] = json::array();
    result["regions"] = json::array();  // Add regions for frontend compatibility

    auto addCategory = [&](const char* type, uint64_t size) {
        if (size > 0) {
            result["categories"].push_back({{"type", type}, {"size", size}});
            result["fragments"].push_back({{"size", size}, {"type", type}});
            result["regions"].push_back({{"type", type}, {"size", size}});
        }
    };

    addCategory("INSTANCE_OBJECT", categoryTotals[0]);
    addCategory("OBJECT_ARRAY", categoryTotals[1]);
    addCategory("STRUCT_ARRAY", categoryTotals[2]);
    addCategory("PRIMITIVE_ARRAY", categoryTotals[3]);
    addCategory("PINNED_OBJECT", pinnedTotal);
    addCategory("LARGE_OBJECT", largeTotal);
    addCategory("FREE_SPACE", freeSpace);

    return result.dump();
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

    json j;
    j["free_total"] = 0;
    j["free_max_continuous"] = 0;
    j["instance_object_total"] = instanceTotal;
    j["object_array_total"] = objectArrayTotal;
    j["struct_array_total"] = structArrayTotal;
    j["primitive_array_total"] = primitiveTotal;
    j["pinned_object_total"] = pinnedTotal;
    j["large_object_total"] = largeTotal;
    j["top10"] = json::array();
    return j.dump();
}

} // namespace cjprof