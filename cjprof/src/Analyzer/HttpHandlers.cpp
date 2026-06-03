// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Analyzer/HttpHandlers.h"
#include "Analyzer/Types.h"
#include "Analyzer/Logger.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <unordered_set>

using json = nlohmann::json;

namespace cjprof {
// kCutoffIdOffset is 1000000000: offset for generating cutoff node IDs (parentId + offset)
static constexpr uint64_t kCutoffIdOffset = 1000000000ULL;
// kMaxObjectIdDisplayCount is 50: max number of object IDs to display in clustered/cutoff nodes
static constexpr size_t kMaxObjectIdDisplayCount = 50;
// kObjectCategoryCount is 7: number of ObjectCategory enum values for array sizing
static constexpr uint8_t kObjectCategoryCount = 7;
// kTop10Count is 10: number of items in top-10 ranking
static constexpr size_t kTop10Count = 10;
// kDefaultHeapLimitBytes is 536870912 (512MB): default heap limit when no snapshot available
static constexpr uint64_t kDefaultHeapLimitBytes = 512ULL * 1024 * 1024;
// kCutoffNodeDepth is 0: depth value assigned to cutoff summary nodes
static constexpr uint32_t kCutoffNodeDepth = 0;
// kMinHeapSize is 1: minimum heap size to avoid division by zero
static constexpr uint64_t kMinHeapSize = 1;


static std::string getClassName(const HttpContext& ctx, uint64_t objectId)
{
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

// Build object_id -> class_name map for ALL objects in one pass (O(N))
static std::unordered_map<uint64_t, std::string> buildObjectIdToClassMap(const HttpContext& ctx)
{
    std::unordered_map<uint64_t, std::string> objectIdToClassMap;
    if (!ctx.objects) return objectIdToClassMap;

    // First build class_id -> class_name map for O(1) lookup
    std::unordered_map<uint64_t, std::string> classIdToNameMap;
    if (ctx.classes) {
        for (const auto& cls : *ctx.classes) {
            if (!cls.class_name.empty()) {
                classIdToNameMap[cls.class_id] = cls.class_name;
            }
        }
    }

    // Then build object_id -> class_name map in one pass
    for (const auto& obj : *ctx.objects) {
        std::string className;
        if (!obj.name.empty()) {
            className = obj.name;
        } else if (obj.class_id == 0) {
            switch (obj.category) {
                case ObjectCategory::PRIMITIVE_ARRAY: className = "PRIMITIVE_ARRAY"; break;
                case ObjectCategory::OBJECT_ARRAY: className = "OBJECT_ARRAY"; break;
                case ObjectCategory::STRUCT_ARRAY: className = "STRUCT_ARRAY"; break;
                case ObjectCategory::PINNED_OBJECT: className = "PINNED_OBJECT"; break;
                case ObjectCategory::LARGE_OBJECT: className = "LARGE_OBJECT"; break;
                case ObjectCategory::UNMOVABLE_OBJECT: className = "UNMOVABLE_OBJECT"; break;
                default: className = "unknown"; break;
            }
        } else {
            auto it = classIdToNameMap.find(obj.class_id);
            className = (it != classIdToNameMap.end()) ? it->second : "unknown";
        }
        objectIdToClassMap[obj.object_id] = className;
    }

    return objectIdToClassMap;
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
    std::unordered_map<std::string, std::vector<const DominanceNode*>> classGroupMap;
    for (const auto* node : children) {
        std::string className = getClassName(ctx, node->object_id);
        classGroupMap[className].push_back(node);
    }

    // Build clustered nodes
    std::vector<ClusterResult> clusteredNodeList;
    for (auto& [className, nodes] : classGroupMap) {
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
        clusteredNodeList.push_back(result);
    }

    // Sort by retained_size descending
    std::sort(clusteredNodeList.begin(), clusteredNodeList.end(), [](const ClusterResult& a, const ClusterResult& b) {
        return a.node.retained_size > b.node.retained_size;
    });

    return clusteredNodeList;
}

std::string HttpHandlers::handleSnapshot(const HttpContext& ctx)
{
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

std::string HttpHandlers::handleDominanceTree(const HttpContext& ctx)
{
    LOG_DEBUG("Handling /api/dominance/tree");

    json result;
    result["nodes"] = json::array();
    result["cutoff_count"] = 0;

    if (!ctx.dominanceNodes) {
        return result.dump();
    }

    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : kMinHeapSize;
    if (usedHeap == 0) usedHeap = kMinHeapSize;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.m_threshold01Percent);
    if (threshold01 == 0) threshold01 = kMinHeapSize;

    std::unordered_map<uint64_t, uint64_t> parentRetainedSizeMap;
    for (const auto& node : *ctx.dominanceNodes) {
        parentRetainedSizeMap[node.object_id] = node.retained_size;
    }

    int cutoffNodeCount = 0;
    std::unordered_map<uint64_t, int> parentCutoffCountMap;
    std::unordered_map<uint64_t, uint64_t> parentCutoffRetainedMap;
    std::unordered_map<uint64_t, uint64_t> parentCutoffShallowMap;

    // Collect root nodes (parent_id == 0 or depth == 0) and apply threshold filtering
    std::vector<const DominanceNode*> rootNodeList;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id == 0 || node.depth == 0) {
            if (node.depth > ctx.m_maxDepthLimit) {
                continue;  // Skip nodes beyond depth limit
            }

            // Apply threshold filtering to root nodes
            if (node.retained_size < threshold01) {
                cutoffNodeCount++;
                parentCutoffCountMap[0]++;  // Use parent_id=0 for root cutoff nodes
                parentCutoffRetainedMap[0] += node.retained_size;
                parentCutoffShallowMap[0] += node.shallow_size;
            } else {
                rootNodeList.push_back(&node);
            }
        }
    }

    // Add root nodes that passed threshold
    for (const auto* node : rootNodeList) {
        std::string className = getClassName(ctx, node->object_id);
        json nodeJson = {
            {"id", node->object_id},
            {"class_name", className},
            {"retained_size", node->retained_size},
            {"shallow_size", node->shallow_size},
            {"depth", node->depth},
            {"parent_id", node->parent_id},
            {"instance_count", 1},
            {"is_clustered", false},
            {"is_cutoff", false}
        };
        result["nodes"].push_back(nodeJson);
    }

    // Group non-root nodes by parent_id
    std::unordered_map<uint64_t, std::vector<const DominanceNode*>> childrenByParentMap;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id == 0 || node.depth == 0) {
            continue;  // Already handled as root nodes
        }
        if (node.depth > ctx.m_maxDepthLimit) {
            continue;  // Skip nodes beyond depth limit
        }
        childrenByParentMap[node.parent_id].push_back(&node);
    }

    // Add non-root nodes with cutoff filtering
    // Only generate cutoff summaries for parents that will appear in the tree
    // (parentRetained >= threshold01). Skip parents that are themselves cutoff.
    for (const auto& [parentId, children] : childrenByParentMap) {
        // Get parent retained size for cutoff calculation
        uint64_t parentRetained = 0;
        auto it = parentRetainedSizeMap.find(parentId);
        if (it != parentRetainedSizeMap.end()) {
            parentRetained = it->second;
        }

        // If parent is itself a cutoff node, it won't appear in the tree.
        // Skip all its children — no cutoff summary needed for invisible parents.
        if (parentRetained < threshold01) {
            cutoffNodeCount += static_cast<int>(children.size());
            continue;
        }

        for (const auto* node : children) {
            // Check if node should be cutoff (either by threshold01 or cutoff05Percent)
            bool isCutoff = false;

            // Nodes below threshold01 are always cutoff
            if (node->retained_size < threshold01) {
                isCutoff = true;
            } else if (parentRetained > 0) {
                // Nodes above threshold01 but below cutoff05Percent are also cutoff
                uint64_t cutoffThreshold = static_cast<uint64_t>(parentRetained * ctx.m_cutoff05Percent);
                if (node->retained_size < cutoffThreshold) {
                    isCutoff = true;
                }
            }

            if (isCutoff) {
                cutoffNodeCount++;
                parentCutoffCountMap[parentId]++;
                parentCutoffRetainedMap[parentId] += node->retained_size;
                parentCutoffShallowMap[parentId] += node->shallow_size;
                continue;
            }

            std::string className = getClassName(ctx, node->object_id);
            json nodeJson = {
                {"id", node->object_id},
                {"class_name", className},
                {"retained_size", node->retained_size},
                {"shallow_size", node->shallow_size},
                {"depth", node->depth},
                {"parent_id", parentId},
                {"instance_count", 1},
                {"is_clustered", false},
                {"is_cutoff", false}
            };
            result["nodes"].push_back(nodeJson);
        }
    }

    for (const auto& entry : parentCutoffCountMap) {
        uint64_t cutoffId = kCutoffIdOffset + entry.first;
        uint64_t cutoffRetained = parentCutoffRetainedMap[entry.first];
        uint64_t cutoffShallow = parentCutoffShallowMap[entry.first];
        result["nodes"].push_back({
            {"id", cutoffId},
            {"class_name", "... (" + std::to_string(entry.second) + " instances)"},
            {"retained_size", cutoffRetained},
            {"shallow_size", cutoffShallow},
            {"depth", kCutoffNodeDepth},
            {"parent_id", entry.first},
            {"instance_count", entry.second},
            {"is_clustered", false},
            {"is_cutoff", true}
        });
    }

    result["cutoff_count"] = cutoffNodeCount;
    return result.dump();
}

std::string HttpHandlers::handleDominanceChildren(const HttpContext& ctx, uint64_t parentId)
{
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

    // Collect children
    std::vector<const DominanceNode*> children;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id == parentId) {
            children.push_back(&node);
        }
    }

    // Add children directly (no clustering)
    for (const auto* node : children) {
        std::string className = getClassName(ctx, node->object_id);
        json nodeJson = {
            {"id", node->object_id},
            {"class_name", className},
            {"retained_size", node->retained_size},
            {"shallow_size", node->shallow_size},
            {"depth", node->depth},
            {"parent_id", parentId},
            {"instance_count", 1},
            {"is_clustered", false},
            {"is_cutoff", false}
        };
        result["nodes"].push_back(nodeJson);
    }

    return result.dump();
}

std::string HttpHandlers::handleDominanceClusterExpand(const HttpContext& ctx, const std::vector<uint64_t>& instanceIds)
{
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

// Hash function for std::pair<std::string, std::string> (used as unordered_map key)
struct PairStringHash {
    std::size_t operator()(const std::pair<std::string, std::string>& p) const {
        return std::hash<std::string>()(p.first) ^ (std::hash<std::string>()(p.second) << 1);
    }
};

std::string HttpHandlers::handleDominanceTreeByType(const HttpContext& ctx)
{
    LOG_DEBUG("Handling /api/dominance/tree-by-type");

    json result;
    result["nodes"] = json::array();
    result["cutoff_count"] = 0;
    result["total_skipped"] = 0;

    if (!ctx.dominanceNodes) {
        return result.dump();
    }

    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : kMinHeapSize;
    if (usedHeap == 0) usedHeap = kMinHeapSize;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.m_threshold01Percent);
    if (threshold01 == 0) threshold01 = kMinHeapSize;

    // TypeNode: keyed by (class_name, parent_type) — per-parent grouping
    // Same type can appear as separate nodes under different parents
    struct TypeNode {
        std::string class_name;
        std::string parent_type;      // The parent type for this grouping
        uint64_t retained_size = 0;
        uint64_t shallow_size = 0;
        uint64_t instance_count = 0;
        std::vector<uint64_t> object_ids;
        std::unordered_set<std::string> child_types;
        int max_depth = 0;
        // Below-threshold children aggregated by their type
        std::unordered_map<std::string, uint64_t> cutoff_type_counts;
        std::unordered_map<std::string, uint64_t> cutoff_type_retained;
        uint64_t cutoff_count = 0;
        uint64_t cutoff_retained = 0;
        uint64_t cutoff_shallow = 0;
    };

    // StandaloneCutoff: types whose all instances are below threshold, keyed by (class_name, parent_type)
    struct StandaloneCutoff {
        std::string class_name;
        std::string parent_type;
        uint64_t retained_size = 0;
        uint64_t shallow_size = 0;
        uint64_t instance_count = 0;
        std::vector<uint64_t> object_ids;
        int max_depth = 0;
    };

    // Build object_id -> class_name map for ALL objects (one pass, O(N))
    auto objectIdToClassMap = buildObjectIdToClassMap(ctx);

    using GroupKey = std::pair<std::string, std::string>;
    std::unordered_map<GroupKey, TypeNode, PairStringHash> typeNodes;
    std::unordered_map<GroupKey, StandaloneCutoff, PairStringHash> standaloneCutoffs;

    // Helper: make node id from (class_name, parent_type)
    auto makeNodeId = [](const std::string& className, const std::string& parentType) -> std::string {
        return className + "@" + parentType;
    };

    // Pass 1: traverse above-threshold objects, group by (class_name, parent_type)
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.retained_size < threshold01) continue;

        auto classIt = objectIdToClassMap.find(node.object_id);
        std::string className = (classIt != objectIdToClassMap.end()) ? classIt->second : "unknown";
        if (className == "unknown") continue;

        // Determine parent type
        std::string parentType = "";
        if (node.parent_id != 0) {
            auto parentIt = objectIdToClassMap.find(node.parent_id);
            if (parentIt != objectIdToClassMap.end()) {
                parentType = parentIt->second;
            }
        }

        GroupKey key = {className, parentType};
        if (typeNodes.find(key) == typeNodes.end()) {
            typeNodes[key] = TypeNode();
            typeNodes[key].class_name = className;
            typeNodes[key].parent_type = parentType;
        }

        TypeNode& tn = typeNodes[key];
        tn.retained_size += node.retained_size;
        tn.shallow_size += node.shallow_size;
        tn.instance_count++;
        tn.object_ids.push_back(node.object_id);
        if (node.depth > tn.max_depth) {
            tn.max_depth = node.depth;
        }

        // Collect child types and aggregate below-threshold children by their type
        for (const auto& child : *ctx.dominanceNodes) {
            if (child.parent_id == node.object_id && child.object_id != node.object_id) {
                auto childClassIt = objectIdToClassMap.find(child.object_id);
                if (childClassIt == objectIdToClassMap.end()) continue;
                std::string childClass = childClassIt->second;
                if (childClass == "unknown") continue;

                if (child.retained_size >= threshold01) {
                    if (childClass != className) {
                        tn.child_types.insert(childClass);
                    }
                } else {
                    tn.cutoff_type_counts[childClass]++;
                    tn.cutoff_type_retained[childClass] += child.retained_size;
                    tn.cutoff_count++;
                    tn.cutoff_retained += child.retained_size;
                    tn.cutoff_shallow += child.shallow_size;
                }
            }
        }
    }

    // Pass 2: traverse below-threshold objects for StandaloneCutoff
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.retained_size >= threshold01) continue;

        auto classIt = objectIdToClassMap.find(node.object_id);
        std::string className = (classIt != objectIdToClassMap.end()) ? classIt->second : "unknown";
        if (className == "unknown") continue;

        std::string parentType = "";
        if (node.parent_id != 0) {
            auto parentIt = objectIdToClassMap.find(node.parent_id);
            if (parentIt != objectIdToClassMap.end()) {
                parentType = parentIt->second;
            }
        }

        GroupKey key = {className, parentType};

        // Skip if this group already has a TypeNode (above-threshold instances exist)
        if (typeNodes.find(key) != typeNodes.end()) continue;

        if (standaloneCutoffs.find(key) == standaloneCutoffs.end()) {
            standaloneCutoffs[key] = StandaloneCutoff();
            standaloneCutoffs[key].class_name = className;
            standaloneCutoffs[key].parent_type = parentType;
        }

        StandaloneCutoff& sc = standaloneCutoffs[key];
        sc.retained_size += node.retained_size;
        sc.shallow_size += node.shallow_size;
        sc.instance_count++;
        sc.object_ids.push_back(node.object_id);
        if (node.depth > sc.max_depth) {
            sc.max_depth = node.depth;
        }
    }

    // Pass 3: build result JSON
    // Generate node id: class_name@parent_type
    // For root-level nodes: class_name@ (empty parent_type part)

    // First, output above-threshold TypeNodes
    for (const auto& [key, tn] : typeNodes) {
        std::string nodeId = makeNodeId(tn.class_name, tn.parent_type);

        // Collect child_types as node IDs (children under this parent)
        json childTypesJson = json::array();
        for (const auto& childTypeName : tn.child_types) {
            // Find child TypeNodes whose parent_type matches this node's class_name
            // and whose class_name matches childTypeName
            for (const auto& [ck, cv] : typeNodes) {
                if (ck.first == childTypeName && ck.second == tn.class_name) {
                    childTypesJson.push_back(makeNodeId(cv.class_name, cv.parent_type));
                }
            }
            // Also check standaloneCutoffs that match
            for (const auto& [sk, sv] : standaloneCutoffs) {
                if (sk.first == childTypeName && sk.second == tn.class_name && sv.retained_size >= threshold01) {
                    childTypesJson.push_back(makeNodeId(sv.class_name + "::cutoff-type", sv.parent_type));
                }
            }
        }
        // Add cutoff type children
        for (const auto& [childType, childRetained] : tn.cutoff_type_retained) {
            if (childRetained >= threshold01) {
                childTypesJson.push_back(makeNodeId(childType + "::cutoff-type", tn.class_name));
            }
        }

        // Compute parent_id: find the node that is this node's parent
        std::string parentId = "";
        if (!tn.parent_type.empty()) {
            // Look for parent in TypeNodes (above-threshold)
            for (const auto& [pk, pv] : typeNodes) {
                if (pk.first == tn.parent_type) {
                    parentId = makeNodeId(pv.class_name, pv.parent_type);
                    break;
                }
            }
            // If not found in TypeNodes, look in StandaloneCutoffs
            if (parentId.empty()) {
                for (const auto& [sk, sv] : standaloneCutoffs) {
                    if (sk.first == tn.parent_type) {
                        parentId = makeNodeId(sv.class_name + "::cutoff-type", sv.parent_type);
                        break;
                    }
                }
            }
        }

        std::string classNameStr = tn.instance_count > 1 ?
            tn.class_name + " (" + std::to_string(tn.instance_count) + " instances)" :
            tn.class_name;
        json nodeJson = {
            {"id", nodeId},
            {"type_name", tn.class_name},
            {"class_name", classNameStr},
            {"retained_size", tn.retained_size},
            {"shallow_size", tn.shallow_size},
            {"depth", tn.max_depth},
            {"parent_type", tn.parent_type},
            {"parent_id", parentId},
            {"instance_count", tn.instance_count},
            {"object_ids", tn.object_ids},
            {"is_clustered", tn.instance_count > 1},
            {"is_cutoff", false},
            {"cutoff_count", tn.cutoff_count},
            {"child_types", childTypesJson}
        };

        result["nodes"].push_back(nodeJson);

        // Cutoff type nodes: below-threshold children aggregated by type, retained >= threshold01
        for (const auto& [childType, childCount] : tn.cutoff_type_counts) {
            auto retIt = tn.cutoff_type_retained.find(childType);
            uint64_t childRetained = retIt != tn.cutoff_type_retained.end() ? retIt->second : 0;
            if (childRetained < threshold01) continue;

            auto scIt = standaloneCutoffs.find({childType, tn.class_name});
            uint64_t childShallow = scIt != standaloneCutoffs.end() ? scIt->second.shallow_size : 0;
            std::vector<uint64_t> childIds =
                scIt != standaloneCutoffs.end() ? scIt->second.object_ids : std::vector<uint64_t>{};
            if (childIds.size() > kMaxObjectIdDisplayCount) childIds.clear();

            std::string cutoffNodeId = makeNodeId(childType + "::cutoff-type", tn.class_name);

            std::string childClassName = childCount > 1 ?
                childType + " (" + std::to_string(childCount) + " instances)" : childType;
            result["nodes"].push_back({
                {"id", cutoffNodeId},
                {"type_name", childType},
                {"class_name", childClassName},
                {"retained_size", childRetained},
                {"shallow_size", childShallow},
                {"depth", tn.max_depth + 1},
                {"parent_type", tn.class_name},
                {"parent_id", nodeId},
                {"instance_count", childCount},
                {"object_ids", childIds},
                {"is_clustered", childCount > 1},
                {"is_cutoff", false},
                {"cutoff_count", 0},
                {"child_types", json::array()}
            });
        }

        // Remaining cutoff: children whose type aggregation didn't reach threshold01
        uint64_t remainingCutoffCount = tn.cutoff_count;
        uint64_t remainingCutoffRetained = tn.cutoff_retained;
        uint64_t remainingCutoffShallow = tn.cutoff_shallow;
        for (const auto& [childType, childCount] : tn.cutoff_type_counts) {
            auto retIt = tn.cutoff_type_retained.find(childType);
            if (retIt != tn.cutoff_type_retained.end() && retIt->second >= threshold01) {
                remainingCutoffCount -= childCount;
                remainingCutoffRetained -= retIt->second;
            }
        }
        if (remainingCutoffCount > 0) {
            result["nodes"].push_back({
                {"id", tn.class_name + "@@cutoff"},
                {"type_name", tn.class_name},
                {"class_name", "... (" + std::to_string(remainingCutoffCount) + " instances)"},
                {"retained_size", remainingCutoffRetained},
                {"shallow_size", remainingCutoffShallow},
                {"depth", tn.max_depth + 1},
                {"parent_type", tn.class_name},
                {"parent_id", nodeId},
                {"instance_count", remainingCutoffCount},
                {"object_ids", json::array()},
                {"is_clustered", false},
                {"is_cutoff", true},
                {"cutoff_count", 0},
                {"child_types", json::array()}
            });
        }
    }

    // StandaloneCutoff nodes at their parent level
    std::unordered_set<GroupKey, PairStringHash> addedStandaloneCutoffs;
    for (const auto& [key, sc] : standaloneCutoffs) {
        if (sc.retained_size < threshold01) continue;
        // Skip if already covered by some TypeNode's cutoff_type_counts
        bool covered = false;
        for (const auto& [tk, tv] : typeNodes) {
            // A StandaloneCutoff (sc.class_name, sc.parent_type) is covered if
            // some TypeNode has sc.class_name in its cutoff_type_counts AND
            // that TypeNode's class_name matches sc.parent_type (because the
            // cutoff children's parent class is the TypeNode itself, not the
            // TypeNode's parent).
            if (tv.cutoff_type_counts.find(sc.class_name) != tv.cutoff_type_counts.end()
                && tk.first == sc.parent_type) {
                covered = true;
                break;
            }
        }
        if (covered) continue;
        if (addedStandaloneCutoffs.count(key)) continue;
        addedStandaloneCutoffs.insert(key);

        std::vector<uint64_t> childIds =
            sc.object_ids.size() <= kMaxObjectIdDisplayCount ? sc.object_ids : std::vector<uint64_t>{};
        std::string nodeId = makeNodeId(sc.class_name + "::cutoff-type", sc.parent_type);

        // Compute parent_id for standalone cutoff
        std::string scParentId = "";
        if (!sc.parent_type.empty()) {
            for (const auto& [pk, pv] : typeNodes) {
                if (pk.first == sc.parent_type) {
                    scParentId = makeNodeId(pv.class_name, pv.parent_type);
                    break;
                }
            }
            if (scParentId.empty()) {
                for (const auto& [sk, sv] : standaloneCutoffs) {
                    if (sk.first == sc.parent_type) {
                        scParentId = makeNodeId(sv.class_name + "::cutoff-type", sv.parent_type);
                        break;
                    }
                }
            }
        }

        // Build child_types for standalone cutoff: other nodes whose parent_type == sc.class_name
        json scChildTypesJson = json::array();
        for (const auto& [ck, cv] : typeNodes) {
            if (cv.parent_type == sc.class_name) {
                scChildTypesJson.push_back(makeNodeId(cv.class_name, cv.parent_type));
            }
        }
        // Check TypeNode cutoff_type_counts whose parent is this standalone cutoff
        for (const auto& [tk, tv] : typeNodes) {
            if (tv.parent_type == sc.class_name) {
                for (const auto& [childType, childRetained] : tv.cutoff_type_retained) {
                    if (childRetained >= threshold01) {
                        scChildTypesJson.push_back(makeNodeId(childType + "::cutoff-type", tv.class_name));
                    }
                }
            }
        }
        // Check other StandaloneCutoffs whose parent_type == sc.class_name
        for (const auto& [sk, sv] : standaloneCutoffs) {
            if (sv.parent_type == sc.class_name && sv.retained_size >= threshold01) {
                scChildTypesJson.push_back(makeNodeId(sv.class_name + "::cutoff-type", sv.parent_type));
            }
        }

        std::string scClassName = sc.instance_count > 1 ?
            sc.class_name + " (" + std::to_string(sc.instance_count) + " instances)" : sc.class_name;
        result["nodes"].push_back({
            {"id", nodeId},
            {"type_name", sc.class_name},
            {"class_name", scClassName},
            {"retained_size", sc.retained_size},
            {"shallow_size", sc.shallow_size},
            {"depth", sc.max_depth},
            {"parent_type", sc.parent_type},
            {"parent_id", scParentId},
            {"instance_count", sc.instance_count},
            {"object_ids", childIds},
            {"is_clustered", sc.instance_count > 1},
            {"is_cutoff", false},
            {"cutoff_count", 0},
            {"child_types", scChildTypesJson}
        });
    }

    // Remaining cutoff: all below-threshold objects not covered by cutoff type nodes
    uint64_t totalBelowThresholdCount = 0;
    uint64_t totalBelowThresholdRetained = 0;
    uint64_t totalBelowThresholdShallow = 0;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.retained_size < threshold01) {
            totalBelowThresholdCount++;
            totalBelowThresholdRetained += node.retained_size;
            totalBelowThresholdShallow += node.shallow_size;
        }
    }
    uint64_t shownCutoffCount = 0;
    uint64_t shownCutoffRetained = 0;
    for (const auto& n : result["nodes"]) {
        if (n["is_cutoff"].get<bool>()) {
            shownCutoffCount += n["instance_count"].get<uint64_t>();
            shownCutoffRetained += n["retained_size"].get<uint64_t>();
        }
    }
    uint64_t remainingCount = totalBelowThresholdCount - shownCutoffCount;
    uint64_t remainingRetained = totalBelowThresholdRetained - shownCutoffRetained;
    if (remainingCount > 0) {
        result["nodes"].push_back({
            {"id", "::remaining-cutoff@"},
            {"type_name", "..."},
            {"class_name", "... (" + std::to_string(remainingCount) + " instances)"},
            {"retained_size", remainingRetained},
            {"shallow_size", totalBelowThresholdShallow},
            {"depth", kCutoffNodeDepth},
            {"parent_type", ""},
            {"instance_count", remainingCount},
            {"object_ids", json::array()},
            {"is_clustered", false},
            {"is_cutoff", true},
            {"cutoff_count", 0},
            {"child_types", json::array()}
        });
    }

    result["cutoff_count"] = totalBelowThresholdCount;
    return result.dump();
}

std::string HttpHandlers::handleDominanceChildrenByType(const HttpContext& ctx, const std::string& nodeId)
{
    LOG_DEBUG("Handling /api/dominance/children-by-type?node_id={}", nodeId);

    json result;
    result["nodes"] = json::array();

    if (!ctx.dominanceNodes) {
        return result.dump();
    }

    // Parse node_id format: "class_name@parent_type" or "class_name@" (root level)
    // For cutoff nodes: "class_name::cutoff-type@parent_type"
    std::string parentClassName;
    std::string parentParentType;  // parent_type of the parent node (for context)
    size_t atPos = nodeId.rfind('@');
    if (atPos != std::string::npos) {
        parentClassName = nodeId.substr(0, atPos);
        parentParentType = nodeId.substr(atPos + 1);
    } else {
        parentClassName = nodeId;
    }

    // Strip ::cutoff-type suffix if present (for expanding cutoff type nodes)
    bool isParentCutoff = false;
    if (parentClassName.find("::cutoff-type") != std::string::npos) {
        isParentCutoff = true;
        parentClassName = parentClassName.substr(0, parentClassName.find("::cutoff-type"));
    }

    // Strip @@cutoff suffix (for expanding cutoff summary nodes)
    if (parentClassName.find("@@cutoff") != std::string::npos) {
        parentClassName = parentClassName.substr(0, parentClassName.find("@@cutoff"));
        // This is a cutoff summary node, no children to expand
        return result.dump();
    }

    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : kMinHeapSize;
    if (usedHeap == 0) usedHeap = kMinHeapSize;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.m_threshold01Percent);
    if (threshold01 == 0) threshold01 = kMinHeapSize;

    // Build object_id -> class_name map for ALL objects (O(N))
    auto objectIdToClassMap = buildObjectIdToClassMap(ctx);

    using GroupKey = std::pair<std::string, std::string>;
    auto makeNodeId = [](const std::string& className, const std::string& parentType) -> std::string {
        return className + "@" + parentType;
    };

    // TypeNode: per-parent grouping
    struct TypeNode {
        std::string class_name;
        std::string parent_type;
        uint64_t retained_size = 0;
        uint64_t shallow_size = 0;
        uint64_t instance_count = 0;
        std::vector<uint64_t> object_ids;
        std::unordered_set<std::string> child_types;
        int max_depth = 0;
        std::unordered_map<std::string, uint64_t> cutoff_type_counts;
        std::unordered_map<std::string, uint64_t> cutoff_type_retained;
        uint64_t cutoff_count = 0;
        uint64_t cutoff_retained = 0;
        uint64_t cutoff_shallow = 0;
    };

    struct StandaloneCutoff {
        std::string class_name;
        std::string parent_type;
        uint64_t retained_size = 0;
        uint64_t shallow_size = 0;
        uint64_t instance_count = 0;
        std::vector<uint64_t> object_ids;
        int max_depth = 0;
    };

    std::unordered_map<GroupKey, TypeNode, PairStringHash> typeNodes;
    std::unordered_map<GroupKey, StandaloneCutoff, PairStringHash> standaloneCutoffs;

    // Find all object_ids belonging to parentClassName with matching parent type
    // These are the instances whose children we want to return
    std::vector<uint64_t> parentObjectIds;

    // Pass 1: build all TypeNodes (above-threshold objects, per-parent grouping)
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.retained_size < threshold01) continue;

        auto classIt = objectIdToClassMap.find(node.object_id);
        std::string className = (classIt != objectIdToClassMap.end()) ? classIt->second : "unknown";
        if (className == "unknown") continue;

        std::string parentType = "";
        if (node.parent_id != 0) {
            auto parentIt = objectIdToClassMap.find(node.parent_id);
            if (parentIt != objectIdToClassMap.end()) {
                parentType = parentIt->second;
            }
        }

        GroupKey key = {className, parentType};
        if (typeNodes.find(key) == typeNodes.end()) {
            typeNodes[key] = TypeNode();
            typeNodes[key].class_name = className;
            typeNodes[key].parent_type = parentType;
        }

        TypeNode& tn = typeNodes[key];
        tn.retained_size += node.retained_size;
        tn.shallow_size += node.shallow_size;
        tn.instance_count++;
        tn.object_ids.push_back(node.object_id);
        if (node.depth > tn.max_depth) {
            tn.max_depth = node.depth;
        }

        // Collect parent object IDs for the queried parent node
        if (className == parentClassName && parentType == parentParentType) {
            parentObjectIds.push_back(node.object_id);
        }

        // Collect child types and cutoff_type_counts
        for (const auto& child : *ctx.dominanceNodes) {
            if (child.parent_id == node.object_id && child.object_id != node.object_id) {
                auto childClassIt = objectIdToClassMap.find(child.object_id);
                if (childClassIt == objectIdToClassMap.end()) continue;
                std::string childClass = childClassIt->second;
                if (childClass == "unknown") continue;

                if (child.retained_size >= threshold01) {
                    if (childClass != className) {
                        tn.child_types.insert(childClass);
                    }
                } else {
                    tn.cutoff_type_counts[childClass]++;
                    tn.cutoff_type_retained[childClass] += child.retained_size;
                    tn.cutoff_count++;
                    tn.cutoff_retained += child.retained_size;
                    tn.cutoff_shallow += child.shallow_size;
                }
            }
        }
    }

    // Pass 2: build StandaloneCutoff for below-threshold objects
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.retained_size >= threshold01) continue;

        auto classIt = objectIdToClassMap.find(node.object_id);
        std::string className = (classIt != objectIdToClassMap.end()) ? classIt->second : "unknown";
        if (className == "unknown") continue;

        std::string parentType = "";
        if (node.parent_id != 0) {
            auto parentIt = objectIdToClassMap.find(node.parent_id);
            if (parentIt != objectIdToClassMap.end()) {
                parentType = parentIt->second;
            }
        }

        // For parent cutoff nodes, also collect below-threshold children
        if (isParentCutoff && parentType == parentClassName) {
            // This below-threshold object is a child of a cutoff type node
            // Its children would be even smaller, skip for now
        }

        GroupKey key = {className, parentType};
        if (typeNodes.find(key) != typeNodes.end()) continue;

        if (standaloneCutoffs.find(key) == standaloneCutoffs.end()) {
            standaloneCutoffs[key] = StandaloneCutoff();
            standaloneCutoffs[key].class_name = className;
            standaloneCutoffs[key].parent_type = parentType;
        }

        StandaloneCutoff& sc = standaloneCutoffs[key];
        sc.retained_size += node.retained_size;
        sc.shallow_size += node.shallow_size;
        sc.instance_count++;
        sc.object_ids.push_back(node.object_id);
        if (node.depth > sc.max_depth) {
            sc.max_depth = node.depth;
        }
    }

    // Find the parent TypeNode
    auto parentIt = typeNodes.find({parentClassName, parentParentType});
    if (parentIt == typeNodes.end() && !isParentCutoff) {
        // Parent not found among above-threshold TypeNodes
        // It might be a StandaloneCutoff — check
        // For now, return empty
    }

    // Return child TypeNodes whose parent_type matches parentClassName and parent_parent matches parentParentType
    // (i.e., children under this specific parent grouping)
    for (const auto& [key, tn] : typeNodes) {
        if (tn.parent_type != parentClassName) continue;

        std::string childNodeId = makeNodeId(tn.class_name, tn.parent_type);

        // Collect child_types for this child node
        json childTypesJson = json::array();
        for (const auto& childTypeName : tn.child_types) {
            for (const auto& [ck, cv] : typeNodes) {
                if (ck.first == childTypeName && ck.second == tn.class_name) {
                    childTypesJson.push_back(makeNodeId(cv.class_name, cv.parent_type));
                }
            }
        }
        for (const auto& [childType, childRetained] : tn.cutoff_type_retained) {
            if (childRetained >= threshold01) {
                childTypesJson.push_back(makeNodeId(childType + "::cutoff-type", tn.class_name));
            }
        }

        std::string classNameStr2 = tn.instance_count > 1 ?
            tn.class_name + " (" + std::to_string(tn.instance_count) + " instances)" : tn.class_name;
        json nodeJson = {
            {"id", childNodeId},
            {"type_name", tn.class_name},
            {"class_name", classNameStr2},
            {"retained_size", tn.retained_size},
            {"shallow_size", tn.shallow_size},
            {"depth", tn.max_depth},
            {"parent_type", tn.parent_type},
            {"instance_count", tn.instance_count},
            {"object_ids", tn.object_ids},
            {"is_clustered", tn.instance_count > 1},
            {"is_cutoff", false},
            {"cutoff_count", tn.cutoff_count},
            {"child_types", childTypesJson}
        };

        result["nodes"].push_back(nodeJson);
    }

    // Return cutoff type children of the parent node
    if (parentIt != typeNodes.end()) {
        const TypeNode& parent = parentIt->second;

        // Cutoff type nodes (aggregated by type, retained >= threshold01)
        for (const auto& [childType, childCount] : parent.cutoff_type_counts) {
            auto retIt = parent.cutoff_type_retained.find(childType);
            uint64_t childRetained = retIt != parent.cutoff_type_retained.end() ? retIt->second : 0;
            if (childRetained < threshold01) continue;

            auto scIt = standaloneCutoffs.find({childType, parentClassName});
            uint64_t childShallow = scIt != standaloneCutoffs.end() ? scIt->second.shallow_size : 0;
            std::vector<uint64_t> childIds =
                scIt != standaloneCutoffs.end() ? scIt->second.object_ids : std::vector<uint64_t>{};
            if (childIds.size() > kMaxObjectIdDisplayCount) childIds.clear();

            std::string cutoffNodeId = makeNodeId(childType + "::cutoff-type", parentClassName);

            std::string childClassName2 = childCount > 1 ?
                childType + " (" + std::to_string(childCount) + " instances)" : childType;
            result["nodes"].push_back({
                {"id", cutoffNodeId},
                {"type_name", childType},
                {"class_name", childClassName2},
                {"retained_size", childRetained},
                {"shallow_size", childShallow},
                {"depth", parent.max_depth + 1},
                {"parent_type", parentClassName},
                {"instance_count", childCount},
                {"object_ids", childIds},
                {"is_clustered", childCount > 1},
                {"is_cutoff", true},
                {"cutoff_count", 0},
                {"child_types", json::array()}
            });
        }

        // Remaining cutoff (children whose type aggregation didn't reach threshold01)
        uint64_t remainingCutoffCount = parent.cutoff_count;
        uint64_t remainingCutoffRetained = parent.cutoff_retained;
        uint64_t remainingCutoffShallow = parent.cutoff_shallow;
        for (const auto& [childType, childCount] : parent.cutoff_type_counts) {
            auto retIt = parent.cutoff_type_retained.find(childType);
            if (retIt != parent.cutoff_type_retained.end() && retIt->second >= threshold01) {
                remainingCutoffCount -= childCount;
                remainingCutoffRetained -= retIt->second;
            }
        }
        if (remainingCutoffCount > 0) {
            result["nodes"].push_back({
                {"id", parentClassName + "@@cutoff"},
                {"type_name", parentClassName},
                {"class_name", "... (" + std::to_string(remainingCutoffCount) + " instances)"},
                {"retained_size", remainingCutoffRetained},
                {"shallow_size", remainingCutoffShallow},
                {"depth", parent.max_depth + 1},
                {"parent_type", parentClassName},
                {"instance_count", remainingCutoffCount},
                {"object_ids", json::array()},
                {"is_clustered", false},
                {"is_cutoff", true},
                {"cutoff_count", 0},
                {"child_types", json::array()}
            });
        }
    }

    // StandaloneCutoff children whose parent_type == parentClassName
    for (const auto& [key, sc] : standaloneCutoffs) {
        if (sc.parent_type != parentClassName) continue;
        if (sc.retained_size < threshold01) continue;
        // Skip if already covered by TypeNode's cutoff_type_counts
        bool covered = false;
        if (parentIt != typeNodes.end()) {
            if (parentIt->second.cutoff_type_counts.find(sc.class_name) != parentIt->second.cutoff_type_counts.end()) {
                covered = true;
            }
        }
        if (covered) continue;

        std::vector<uint64_t> childIds =
            sc.object_ids.size() <= kMaxObjectIdDisplayCount ? sc.object_ids : std::vector<uint64_t>{};
        std::string scNodeId = makeNodeId(sc.class_name + "::cutoff-type", sc.parent_type);

        std::string scClassName2 = sc.instance_count > 1 ?
            sc.class_name + " (" + std::to_string(sc.instance_count) + " instances)" : sc.class_name;
        result["nodes"].push_back({
            {"id", scNodeId},
            {"type_name", sc.class_name},
            {"class_name", scClassName2},
            {"retained_size", sc.retained_size},
            {"shallow_size", sc.shallow_size},
            {"depth", sc.max_depth},
            {"parent_type", sc.parent_type},
            {"instance_count", sc.instance_count},
            {"object_ids", childIds},
            {"is_clustered", sc.instance_count > 1},
            {"is_cutoff", true},
            {"cutoff_count", 0},
            {"child_types", json::array()}
        });
    }

    return result.dump();
}

std::string HttpHandlers::handleDominanceTop10(const HttpContext& ctx)
{
    LOG_DEBUG("Handling /api/dominance/top10");

    json result;
    result["items"] = json::array();

    if (!ctx.dominanceNodes) {
        return result.dump();
    }

    std::vector<const DominanceNode*> sortedNodes;
    size_t totalNodes = ctx.dominanceNodes->size();

    // Initialize sortedNodes with first 10 nodes (or all nodes if less than 10)
    size_t initialCount = std::min(kTop10Count, totalNodes);
    for (size_t i = 0; i < initialCount; ++i) {
        sortedNodes.push_back(&ctx.dominanceNodes->at(i));
    }

    // Process remaining nodes: replace smallest node in sortedNodes if current node is larger
    for (size_t i = initialCount; i < totalNodes; ++i) {
        const DominanceNode* current = &ctx.dominanceNodes->at(i);

        // Find node with smallest retained_size in sortedNodes
        auto minIt = std::min_element(sortedNodes.begin(), sortedNodes.end(),
            [](const DominanceNode* a, const DominanceNode* b) {
                return a->retained_size < b->retained_size;
            });

        // Replace if current node is larger than the smallest node
        if (current->retained_size > (*minIt)->retained_size) {
            *minIt = current;
        }
    }

    // Sort by retained_size descending
    std::sort(sortedNodes.begin(), sortedNodes.end(), [](const DominanceNode* a, const DominanceNode* b) {
        return a->retained_size > b->retained_size;
    });

    uint64_t totalSize = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : kMinHeapSize;
    if (totalSize == 0) totalSize = kMinHeapSize;

    int rank = 0;
    for (size_t i = 0; i < sortedNodes.size() && rank < kTop10Count; i++, rank++) {
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

std::string HttpHandlers::handleFragmentOverview(const HttpContext& ctx)
{
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

std::string HttpHandlers::handleFragmentLayout(const HttpContext& ctx)
{
    LOG_DEBUG("Handling /api/fragment/layout");

    uint64_t categoryTotals[kObjectCategoryCount] = {0};

    if (ctx.objects) {
        for (const auto& obj : *ctx.objects) {
            uint8_t catIndex = static_cast<uint8_t>(obj.category);
            if (catIndex < kObjectCategoryCount) {
                categoryTotals[catIndex] += obj.size;
            }
        }
    }

    uint64_t heapLimit = ctx.snapshotInfo ? ctx.snapshotInfo->heap_total_size : kDefaultHeapLimitBytes;
    uint64_t usedSize = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 0;
    uint64_t freeSpace = (heapLimit > usedSize) ? (heapLimit - usedSize) : 0;

    uint64_t pinnedTotal =
        categoryTotals[static_cast<uint8_t>(ObjectCategory::PINNED_OBJECT)]
        + categoryTotals[static_cast<uint8_t>(ObjectCategory::UNMOVABLE_OBJECT)];
    uint64_t largeTotal = categoryTotals[static_cast<uint8_t>(ObjectCategory::LARGE_OBJECT)];

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

std::string HttpHandlers::handleFragmentSummary(const HttpContext& ctx)
{
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