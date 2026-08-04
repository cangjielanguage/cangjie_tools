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
#include <sstream>
#include <iomanip>
#include <unordered_set>

using json = nlohmann::json;

namespace cjprof {
// K_CUTOFF_ID_OFFSET is 1000000000: offset for generating cutoff node IDs (parentId + offset)
static constexpr uint64_t K_CUTOFF_ID_OFFSET = 1000000000ULL;
// K_MAX_OBJECT_ID_DISPLAY_COUNT is 50: max number of object IDs to display in clustered/cutoff nodes
static constexpr size_t K_MAX_OBJECT_ID_DISPLAY_COUNT = 50;
// K_OBJECT_CATEGORY_COUNT is 7: number of ObjectCategory enum values for array sizing
static constexpr uint8_t K_OBJECT_CATEGORY_COUNT = 7;
// K_TOP10_COUNT is 10: number of items in top-10 ranking
static constexpr size_t K_TOP10_COUNT = 10;
// K_DEFAULT_HEAP_LIMIT_BYTES is 536870912 (512MB): default heap limit when no snapshot available
static constexpr uint64_t K_DEFAULT_HEAP_LIMIT_BYTES = 512ULL * 1024 * 1024;
// K_CUTOFF_NODE_DEPTH is 0: depth value assigned to cutoff summary nodes
static constexpr uint32_t K_CUTOFF_NODE_DEPTH = 0;
// K_MIN_HEAP_SIZE is 1: minimum heap size to avoid division by zero
static constexpr uint64_t K_MIN_HEAP_SIZE = 1;
// K_BUCKET_SIZE is 65536 (64KB): address bucket width for the heap card grid
static constexpr uint64_t K_BUCKET_SIZE = 64 * 1024;
// K_ROW_SIZE is 67108864 (64MB): address width of one distribution row
static constexpr uint64_t K_ROW_SIZE = 64ULL * 1024 * 1024;

// categoryToString: convert ObjectCategory enum to string for JSON output
static std::string categoryToString(ObjectCategory cat)
{
    switch (cat) {
        case ObjectCategory::INSTANCE_OBJECT:  return "INSTANCE_OBJECT";
        case ObjectCategory::OBJECT_ARRAY:     return "OBJECT_ARRAY";
        case ObjectCategory::STRUCT_ARRAY:     return "STRUCT_ARRAY";
        case ObjectCategory::PRIMITIVE_ARRAY:  return "PRIMITIVE_ARRAY";
        case ObjectCategory::PINNED_OBJECT:    return "PINNED_OBJECT";
        case ObjectCategory::LARGE_OBJECT:     return "LARGE_OBJECT";
        case ObjectCategory::UNMOVABLE_OBJECT: return "UNMOVABLE_OBJECT";
        default:                               return "UNKNOWN";
    }
}

// HexStr: format uint64_t as hex string "0x..."
static std::string HexStr(uint64_t val)
{
    std::ostringstream oss;
    oss << "0x" << std::hex << val;
    return oss.str();
}

// BucketInfo: dominant ObjectCategory of one 64KB bucket plus the cumulative
// size of all objects that fell into it. totalObjSize==0 marks an empty bucket,
// which mergeSegments renders as FREE_SPACE.
struct BucketInfo {
    ObjectCategory category;
    uint64_t totalObjSize;
};

// Segment: a contiguous address run of a single type, produced by merging
// adjacent same-category buckets. typeName is the category name string
// ("INSTANCE_OBJECT", ... , or "FREE_SPACE").
struct Segment {
    std::string typeName;
    uint64_t startAddr;
    uint64_t endAddr;
};

// buildBuckets: divide [rangeStart, rangeEnd] into 64KB buckets and accumulate
// per-category sizes from the address-sorted objects. Then pick the dominant
// category per bucket — the largest cumulative size wins, strict > so a tie
// goes to the lower enum value (e.g. PINNED beats LARGE when equal). Empty
// buckets keep INSTANCE_OBJECT as a placeholder category; the caller relies
// on totalObjSize==0 (not category) to treat them as free.
static std::vector<BucketInfo> buildBuckets(const std::vector<const HeapObject*>& sorted,
                                            uint64_t rangeStart, uint64_t rangeEnd,
                                            uint64_t totalRange)
{
    size_t bucketCount = std::max<size_t>(1, static_cast<size_t>((totalRange + K_BUCKET_SIZE - 1) / K_BUCKET_SIZE));

    // Per-category totals per bucket: flat array [bucket * K_OBJECT_CATEGORY_COUNT + catIdx]
    std::vector<uint64_t> bucketTotals(bucketCount * K_OBJECT_CATEGORY_COUNT, 0);

    for (const auto* obj : sorted) {
        uint64_t addr = obj->object_id;
        size_t bIdx = static_cast<size_t>((addr - rangeStart) / K_BUCKET_SIZE);
        if (bIdx >= bucketCount) bIdx = bucketCount - 1;
        ObjectCategory cat = obj->category;
        uint8_t catIdx = static_cast<uint8_t>(cat);
        if (catIdx < K_OBJECT_CATEGORY_COUNT) {
            bucketTotals[bIdx * K_OBJECT_CATEGORY_COUNT + catIdx] += obj->size;
        }
    }

    // Determine dominant category per bucket (largest cumulative size wins)
    std::vector<BucketInfo> buckets(bucketCount);
    for (size_t i = 0; i < bucketCount; ++i) {
        uint64_t dominantCatSize = 0;
        uint8_t dominantCatIdx = 0;
        uint64_t totalSize = 0;
        for (uint8_t catIdx = 0; catIdx < K_OBJECT_CATEGORY_COUNT; ++catIdx) {
            uint64_t catSize = bucketTotals[i * K_OBJECT_CATEGORY_COUNT + catIdx];
            totalSize += catSize;
            if (catSize > dominantCatSize) {
                dominantCatSize = catSize;
                dominantCatIdx = catIdx;
            }
        }
        buckets[i].totalObjSize = totalSize;
        buckets[i].category = (totalSize > 0) ? static_cast<ObjectCategory>(dominantCatIdx)
                                               : ObjectCategory::INSTANCE_OBJECT;
    }
    return buckets;
}

// mergeSegments: walk adjacent buckets and emit a Segment whenever the
// (free?, category) pair changes, so a run of identical buckets collapses into
// one Segment. Close the final segment at rangeEnd, and append a trailing
// FREE_SPACE segment if the last segment ends before rangeEnd (this trailing
// branch is currently unreachable because bucketCount uses ceiling division,
// but is kept for correctness/robustness).
static std::vector<Segment> mergeSegments(const std::vector<BucketInfo>& buckets,
                                          uint64_t rangeStart, uint64_t rangeEnd,
                                          uint64_t totalRange)
{
    size_t bucketCount = buckets.size();

    std::vector<Segment> segments;

    bool curIsFree = (buckets[0].totalObjSize == 0);
    ObjectCategory curCat = curIsFree ? ObjectCategory::INSTANCE_OBJECT : buckets[0].category;
    uint64_t curStart = rangeStart;

    for (size_t i = 1; i < bucketCount; ++i) {
        bool isFree = (buckets[i].totalObjSize == 0);
        ObjectCategory cat = isFree ? ObjectCategory::INSTANCE_OBJECT : buckets[i].category;

        if (isFree == curIsFree && cat == curCat) {
            continue;  // same type, keep merging
        }

        uint64_t curEnd = rangeStart + i * K_BUCKET_SIZE;
        segments.push_back({
            curIsFree ? "FREE_SPACE" : categoryToString(curCat),
            curStart, curEnd
        });
        curStart = curEnd;
        curCat = cat;
        curIsFree = isFree;
    }

    // Close last segment
    uint64_t lastEnd = rangeStart + bucketCount * K_BUCKET_SIZE;
    if (lastEnd > rangeEnd) {
        lastEnd = rangeEnd;
    }
    segments.push_back({
        curIsFree ? "FREE_SPACE" : categoryToString(curCat),
        curStart, lastEnd
    });

    // Trailing free space
    if (segments.back().endAddr < rangeEnd) {
        segments.push_back({"FREE_SPACE", segments.back().endAddr, rangeEnd});
    }

    return segments;
}

// groupIntoRows: bin segments into 64MB rows. A segment that straddles a row
// boundary is split at each boundary it spans, so every returned row owns only
// segments fully inside its [rowStart, rowEnd] range.
static std::vector<std::vector<Segment>> groupIntoRows(const std::vector<Segment>& segments,
                                                       uint64_t rangeStart, uint64_t rangeEnd,
                                                       uint64_t totalRange)
{
    size_t rowCount = static_cast<size_t>((totalRange + K_ROW_SIZE - 1) / K_ROW_SIZE);
    if (rowCount == 0) rowCount = 1;
    std::vector<std::vector<Segment>> rows(rowCount);

    for (const auto& seg : segments) {
        size_t startRow = static_cast<size_t>((seg.startAddr - rangeStart) / K_ROW_SIZE);
        size_t endRow = static_cast<size_t>((seg.endAddr - 1 - rangeStart) / K_ROW_SIZE);
        if (endRow >= rowCount) endRow = rowCount - 1;

        if (startRow == endRow) {
            rows[startRow].push_back(seg);
        } else {
            // Segment crosses row boundary — split it
            for (size_t r = startRow; r <= endRow; ++r) {
                uint64_t rowStartAddr = rangeStart + r * K_ROW_SIZE;
                uint64_t rowEndAddr = std::min(rangeStart + (r + 1) * K_ROW_SIZE, rangeEnd);
                uint64_t segStartInRow = std::max(seg.startAddr, rowStartAddr);
                uint64_t segEndInRow = std::min(seg.endAddr, rowEndAddr);
                rows[r].push_back({seg.typeName, segStartInRow, segEndInRow});
            }
        }
    }
    return rows;
}

// buildDistributionJson: assemble the /api/fragment/distribution response —
// top-level address_range_* (hex) + total_address_range, then rows[], each
// row with its segments carrying type/start_address/end_address/size/percentage
// (percentage is the segment's share of its row's size, rounded to 2 dp).
static json buildDistributionJson(uint64_t rangeStart, uint64_t rangeEnd,
                                  uint64_t totalRange,
                                  const std::vector<std::vector<Segment>>& rows)
{
    size_t rowCount = rows.size();

    json result;
    result["address_range_start"] = HexStr(rangeStart);
    result["address_range_end"] = HexStr(rangeEnd);
    result["total_address_range"] = totalRange;

    result["rows"] = json::array();
    for (size_t r = 0; r < rowCount; ++r) {
        uint64_t rowStartAddr = rangeStart + r * K_ROW_SIZE;
        uint64_t rowEndAddr = std::min(rangeStart + (r + 1) * K_ROW_SIZE, rangeEnd);
        uint64_t rowTotalSize = rowEndAddr - rowStartAddr;

        json rowJson;
        rowJson["row_index"] = r;
        rowJson["start_address"] = HexStr(rowStartAddr);
        rowJson["end_address"] = HexStr(rowEndAddr);
        rowJson["size"] = rowTotalSize;
        rowJson["segments"] = json::array();

        for (const auto& seg : rows[r]) {
            uint64_t segSize = seg.endAddr - seg.startAddr;
            double pct = (rowTotalSize > 0) ? ((double)segSize * 100.0 / (double)rowTotalSize) : 0.0;
            pct = std::round(pct * 100.0) / 100.0;

            rowJson["segments"].push_back({
                {"type", seg.typeName},
                {"start_address", HexStr(seg.startAddr)},
                {"end_address", HexStr(seg.endAddr)},
                {"size", segSize},
                {"percentage", pct}
            });
        }
        result["rows"].push_back(rowJson);
    }

    return result;
}

// buildEmptyFallback: distribution response when every object was filtered out
// (sorted.empty()) — a single FREE_SPACE segment spanning the whole address
// range. Segments must be objects ({"type":...}) matching the main path and
// what the frontend chart reads (seg.type / seg.start_address / seg.size /
// seg.percentage). json::array({{pair},{pair}}) would wrongly emit
// arrays-of-arrays, so build the object first then wrap: json::array({seg}).
static json buildEmptyFallback(uint64_t rangeStart, uint64_t rangeEnd, uint64_t totalRange)
{
    json j;
    j["address_range_start"] = HexStr(rangeStart);
    j["address_range_end"] = HexStr(rangeEnd);
    j["total_address_range"] = totalRange;
    json seg = {
        {"type", "FREE_SPACE"},
        {"start_address", HexStr(rangeStart)},
        {"end_address", HexStr(rangeEnd)},
        {"size", totalRange},
        {"percentage", 100.0}
    };
    j["rows"] = json::array();
    j["rows"].push_back({
        {"row_index", 0},
        {"start_address", HexStr(rangeStart)},
        {"end_address", HexStr(rangeEnd)},
        {"size", totalRange},
        {"segments", json::array({seg})}
    });
    return j;
}


static std::string getClassName(const HttpContext& ctx, uint64_t objectId)
{
    auto it = ctx.objectIdToClassName.find(objectId);
    if (it != ctx.objectIdToClassName.end()) {
        return it->second;
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
        ctx.typeTreeJson = result.dump();
        ctx.typeTreeChildrenByParentId.clear();
        ctx.typeTreeBuilt = true;
        return ctx.typeTreeJson;
    }

    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : K_MIN_HEAP_SIZE;
    if (usedHeap == 0) usedHeap = K_MIN_HEAP_SIZE;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.m_threshold01Percent);
    if (threshold01 == 0) threshold01 = K_MIN_HEAP_SIZE;

    // Use pre-built index instead of building parentRetainedSizeMap from scratch
    const auto& parentRetainedSizeMap = ctx.objectIdToRetainedSize;

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

    // Use pre-built childrenByParentId index instead of building childrenByParentMap from scratch
    // Only process parents that have children in the index
    for (const auto& [parentId, children] : ctx.childrenByParentId) {
        // Get parent retained size for cutoff calculation — O(1) via pre-built index
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
            // Skip root nodes (they are handled above)
            if (node->parent_id == 0 || node->depth == 0) {
                continue;
            }

            if (node->depth > ctx.m_maxDepthLimit) {
                continue;  // Skip nodes beyond depth limit
            }

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
        uint64_t cutoffId = K_CUTOFF_ID_OFFSET + entry.first;
        uint64_t cutoffRetained = parentCutoffRetainedMap[entry.first];
        uint64_t cutoffShallow = parentCutoffShallowMap[entry.first];
        result["nodes"].push_back({
            {"id", cutoffId},
            {"class_name", "... (" + std::to_string(entry.second) + " instances)"},
            {"retained_size", cutoffRetained},
            {"shallow_size", cutoffShallow},
            {"depth", K_CUTOFF_NODE_DEPTH},
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

    // Find parent's retained size — O(1) via pre-built index
    uint64_t parentRetained = 0;
    auto retainedIt = ctx.objectIdToRetainedSize.find(parentId);
    if (retainedIt != ctx.objectIdToRetainedSize.end()) {
        parentRetained = retainedIt->second;
    }

    // Collect children — O(children_count) via pre-built index instead of O(N) scan
    auto childIt = ctx.childrenByParentId.find(parentId);
    if (childIt != ctx.childrenByParentId.end()) {
        for (const auto* node : childIt->second) {
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
    // Use pre-built indexes for O(1) lookup instead of O(N) scan per instance
    for (const auto& instanceId : instanceIds) {
        // Check if this instance has children — O(1) via childrenByParentId index
        auto childIt = ctx.childrenByParentId.find(instanceId);
        bool hasChildren = (childIt != ctx.childrenByParentId.end() && !childIt->second.empty());

        // Find node info via objectIdToRetainedSize to confirm existence, then scan for full details
        // (dominanceNodes has no object_id index, so we still need a scan for the full node data)
        auto retainedIt = ctx.objectIdToRetainedSize.find(instanceId);
        if (retainedIt == ctx.objectIdToRetainedSize.end()) {
            continue;  // Instance not found in dominanceNodes
        }

        // Find the full node data — O(N) but only scanning until found
        for (const auto& node : *ctx.dominanceNodes) {
            if (node.object_id == instanceId) {
                std::string className = getClassName(ctx, node.object_id);
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

    uint64_t usedHeap = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : K_MIN_HEAP_SIZE;
    if (usedHeap == 0) usedHeap = K_MIN_HEAP_SIZE;
    uint64_t threshold01 = static_cast<uint64_t>(usedHeap * ctx.m_threshold01Percent);
    if (threshold01 == 0) threshold01 = K_MIN_HEAP_SIZE;

    const auto& objectIdToClassMap = ctx.objectIdToClassName;

    // -------------------------------------------------------------------------
    // Step 1: BFS top-down build the COMPLETE logical type tree.
    // Each object is grouped by (parentLogicalNodeId, class_name).
    // Because the object tree is a tree, this gives a unique logical type node
    // per (parent logical node, child class) pair.
    // All internal IDs are uint64_t; strings are only generated for JSON output.
    // -------------------------------------------------------------------------
    struct LogicalTypeNode {
        uint64_t id = 0;                 // Numeric node ID
        uint64_t parent_id = 0;          // Parent numeric ID (0 for roots)
        std::string class_name;
        std::string parent_type;         // Parent class name
        uint64_t retained_size = 0;
        uint64_t shallow_size = 0;
        uint64_t instance_count = 0;
        std::vector<uint64_t> object_ids;
        int max_depth = 0;
        std::vector<uint64_t> child_ids; // Child numeric IDs
    };

    std::unordered_map<std::string, std::unique_ptr<LogicalTypeNode>> logicalNodes;
    std::unordered_map<uint64_t, LogicalTypeNode*> objectToLogicalNode;
    uint64_t nextNodeId = 1;

    auto getClassName = [&](uint64_t objectId) -> std::string {
        auto it = objectIdToClassMap.find(objectId);
        if (it != objectIdToClassMap.end() && !it->second.empty()) {
            return it->second;
        }
        return "unknown";
    };

    // Collect root objects.
    std::vector<uint64_t> currentLevel;
    for (const auto& node : *ctx.dominanceNodes) {
        if (node.parent_id == 0) {
            currentLevel.push_back(node.object_id);
        }
    }

    // BFS level by level.
    while (!currentLevel.empty()) {
        std::vector<uint64_t> nextLevel;
        for (uint64_t objectId : currentLevel) {
            auto dnIt = ctx.objectIdToDominanceNode.find(objectId);
            if (dnIt == ctx.objectIdToDominanceNode.end()) continue;
            const DominanceNode* dNode = dnIt->second;

            std::string className = getClassName(objectId);

            LogicalTypeNode* parentLogical = nullptr;
            uint64_t parentId = 0;
            std::string parentType;
            if (dNode->parent_id != 0) {
                auto parentLogicalIt = objectToLogicalNode.find(dNode->parent_id);
                if (parentLogicalIt != objectToLogicalNode.end()) {
                    parentLogical = parentLogicalIt->second;
                    parentId = parentLogical->id;
                    parentType = parentLogical->class_name;
                }
            }

            // Aggregation key: (parent_numeric_id, class_name). Use '\0' separator
            // to avoid collisions with class names containing special characters.
            std::string aggKey = className + std::string(1, '\0') + std::to_string(parentId);

            LogicalTypeNode* logical = nullptr;
            auto lnIt = logicalNodes.find(aggKey);
            if (lnIt == logicalNodes.end()) {
                auto newNode = std::make_unique<LogicalTypeNode>();
                logical = newNode.get();
                logical->id = nextNodeId++;
                logical->class_name = className;
                logical->parent_id = parentId;
                logical->parent_type = parentType;
                logicalNodes[aggKey] = std::move(newNode);
                if (parentLogical) {
                    parentLogical->child_ids.push_back(logical->id);
                }
            } else {
                logical = lnIt->second.get();
            }

            logical->retained_size += dNode->retained_size;
            logical->shallow_size += dNode->shallow_size;
            logical->instance_count += 1;
            logical->object_ids.push_back(objectId);
            if (dNode->depth > logical->max_depth) {
                logical->max_depth = dNode->depth;
            }
            objectToLogicalNode[objectId] = logical;

            // Enqueue children for next level using the pre-built index.
            auto childIt = ctx.childrenByParentId.find(objectId);
            if (childIt != ctx.childrenByParentId.end()) {
                for (const DominanceNode* childNode : childIt->second) {
                    nextLevel.push_back(childNode->object_id);
                }
            }
        }
        currentLevel.swap(nextLevel);
    }

    // -------------------------------------------------------------------------
    // Step 2: Decide which logical nodes to emit as TypeNodes and which to
    // collapse into a single cutoff child per parent.
    // -------------------------------------------------------------------------
    std::unordered_set<uint64_t> emittedNodeIds;
    std::unordered_map<uint64_t, LogicalTypeNode*> emittedIdToNode;

    // Mark above-threshold nodes as emitted.
    for (auto& kv : logicalNodes) {
        LogicalTypeNode* node = kv.second.get();
        if (node->retained_size >= threshold01) {
            emittedNodeIds.insert(node->id);
            emittedIdToNode[node->id] = node;
        }
    }

    // For each below-threshold logical node, aggregate it into its parent node's
    // cutoff. The whole subtree below an emitted node is collapsed into one
    // cutoff child, regardless of the concrete class names inside.
    struct CutoffAggregate {
        uint64_t retained_size = 0;
        uint64_t shallow_size = 0;
        uint64_t instance_count = 0;
    };
    std::unordered_map<uint64_t, CutoffAggregate> cutoffAggregates;

    for (auto& kv : logicalNodes) {
        LogicalTypeNode* node = kv.second.get();
        if (emittedNodeIds.count(node->id)) continue;

        if (node->parent_id != 0 && emittedNodeIds.count(node->parent_id)) {
            // Direct below-threshold child of an emitted node: collapse into parent's cutoff.
            CutoffAggregate& agg = cutoffAggregates[node->parent_id];
            agg.retained_size += node->retained_size;
            agg.shallow_size += node->shallow_size;
            agg.instance_count += node->instance_count;
        } else if (node->parent_id == 0) {
            // Below-threshold root node: collapse into the global root cutoff.
            CutoffAggregate& agg = cutoffAggregates[0];
            agg.retained_size += node->retained_size;
            agg.shallow_size += node->shallow_size;
            agg.instance_count += node->instance_count;
        }
        // Nodes whose parent is also below-threshold are implicitly included in
        // the parent's retained_size, so they do not need separate aggregation.
    }

    // -------------------------------------------------------------------------
    // Step 3: Emit nodes.
    // -------------------------------------------------------------------------
    uint64_t totalBelowThresholdCount = 0;

    // Helper to emit a logical type node.
    auto emitLogicalNode = [&](LogicalTypeNode* node) {
        std::string classNameStr = node->instance_count > 1
            ? node->class_name + " (" + std::to_string(node->instance_count) + " instances)"
            : node->class_name;

        std::vector<uint64_t> objectIds = node->object_ids;
        if (objectIds.size() > K_MAX_OBJECT_ID_DISPLAY_COUNT) {
            objectIds.clear();
        }

        json nodeJson = {
            {"id", std::to_string(node->id)},
            {"type_name", node->class_name},
            {"class_name", classNameStr},
            {"retained_size", node->retained_size},
            {"shallow_size", node->shallow_size},
            {"depth", node->max_depth},
            {"parent_type", node->parent_type},
            {"parent_id", node->parent_id == 0 ? "" : std::to_string(node->parent_id)},
            {"instance_count", node->instance_count},
            {"object_ids", objectIds},
            {"is_clustered", node->instance_count > 1},
            {"is_cutoff", false},
            {"cutoff_count", 0},
            {"child_types", json::array()}
        };
        result["nodes"].push_back(nodeJson);
    };

    // Emit above-threshold logical nodes in BFS order for stable output.
    std::vector<LogicalTypeNode*> emittedNodes;
    for (const auto& id : emittedNodeIds) {
        emittedNodes.push_back(emittedIdToNode[id]);
    }
    std::sort(emittedNodes.begin(), emittedNodes.end(), [](LogicalTypeNode* a, LogicalTypeNode* b) {
        if (a->max_depth != b->max_depth) return a->max_depth < b->max_depth;
        return a->id < b->id;
    });

    for (LogicalTypeNode* node : emittedNodes) {
        emitLogicalNode(node);
    }

    // Emit a single cutoff node per parent that has below-threshold children.
    for (const auto& parentKv : cutoffAggregates) {
        uint64_t parentId = parentKv.first;
        const CutoffAggregate& agg = parentKv.second;
        if (agg.instance_count == 0) continue;

        totalBelowThresholdCount += agg.instance_count;

        uint64_t cutoffId = nextNodeId++;

        std::string parentType;
        if (parentId != 0) {
            auto it = emittedIdToNode.find(parentId);
            if (it != emittedIdToNode.end()) {
                parentType = it->second->class_name;
            }
        }

        result["nodes"].push_back({
            {"id", std::to_string(cutoffId)},
            {"type_name", "..."},
            {"class_name", "... (" + std::to_string(agg.instance_count) + " instances)"},
            {"retained_size", agg.retained_size},
            {"shallow_size", agg.shallow_size},
            {"depth", K_CUTOFF_NODE_DEPTH},
            {"parent_type", parentType},
            {"parent_id", parentId == 0 ? "" : std::to_string(parentId)},
            {"instance_count", agg.instance_count},
            {"object_ids", json::array()},
            {"is_clustered", false},
            {"is_cutoff", true},
            {"cutoff_count", 0},
            {"child_types", json::array()}
        });
    }

    result["cutoff_count"] = totalBelowThresholdCount;

    // -------------------------------------------------------------------------
    // Step 4: Fill child_types for every emitted node so the frontend tree view
    // knows which nodes are expandable.
    // -------------------------------------------------------------------------
    std::unordered_map<std::string, std::vector<std::string>> outputChildrenByParent;
    for (const auto& node : result["nodes"]) {
        std::string parentId = node.value("parent_id", "");
        outputChildrenByParent[parentId].push_back(node.value("id", ""));
    }
    for (auto& node : result["nodes"]) {
        std::string nodeId = node.value("id", "");
        node["child_types"] = outputChildrenByParent[nodeId];
    }

    // -------------------------------------------------------------------------
    // Step 5: Build shared cache for children-by-type.
    // -------------------------------------------------------------------------
    ctx.typeTreeJson = result.dump();
    ctx.typeTreeChildrenByParentId.clear();
    for (const auto& node : result["nodes"]) {
        std::string parentId = node.value("parent_id", "");
        ctx.typeTreeChildrenByParentId[parentId].push_back(node);
    }
    ctx.typeTreeBuilt = true;

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

    // Build the shared type-tree cache (no-op if already built) so that this endpoint
    // returns exactly the children declared by /dominance/tree-by-type.
    handleDominanceTreeByType(ctx);

    auto it = ctx.typeTreeChildrenByParentId.find(nodeId);
    if (it != ctx.typeTreeChildrenByParentId.end()) {
        result["nodes"] = it->second;
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
    size_t initialCount = std::min(K_TOP10_COUNT, totalNodes);
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

    uint64_t totalSize = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : K_MIN_HEAP_SIZE;
    if (totalSize == 0) totalSize = K_MIN_HEAP_SIZE;

    int rank = 0;
    for (size_t i = 0; i < sortedNodes.size() && rank < K_TOP10_COUNT; i++, rank++) {
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
        j["address_range_start"] = ctx.snapshotInfo->address_range_start;
        j["address_range_end"] = ctx.snapshotInfo->address_range_end;
    } else {
        j = {{"heap_limit", 0}, {"used_size", 0}, {"utilization", 0.0},
            {"address_range_start", 0}, {"address_range_end", 0}};
    }
    return j.dump();
}

std::string HttpHandlers::handleFragmentLayout(const HttpContext& ctx)
{
    LOG_DEBUG("Handling /api/fragment/layout");

    uint64_t categoryTotals[K_OBJECT_CATEGORY_COUNT] = {0};

    if (ctx.objects) {
        for (const auto& obj : *ctx.objects) {
            uint8_t catIndex = static_cast<uint8_t>(obj.category);
            if (catIndex < K_OBJECT_CATEGORY_COUNT) {
                categoryTotals[catIndex] += obj.size;
            }
        }
    }

    uint64_t heapLimit = ctx.snapshotInfo ? ctx.snapshotInfo->heap_total_size : K_DEFAULT_HEAP_LIMIT_BYTES;
    uint64_t usedSize = ctx.snapshotInfo ? ctx.snapshotInfo->used_size : 0;
    uint64_t freeSpace = (heapLimit > usedSize) ? (heapLimit - usedSize) : 0;

    uint64_t pinnedTotal = categoryTotals[static_cast<uint8_t>(ObjectCategory::PINNED_OBJECT)];
    uint64_t largeTotal = categoryTotals[static_cast<uint8_t>(ObjectCategory::LARGE_OBJECT)];
    uint64_t unmovableTotal = categoryTotals[static_cast<uint8_t>(ObjectCategory::UNMOVABLE_OBJECT)];

    json result;
    result["categories"] = json::array();
    result["fragments"] = json::array();
    result["regions"] = json::array();  // Add regions for frontend compatibility

    auto addCategory = [&](const char* type, uint64_t size) {
        result["categories"].push_back({{"type", type}, {"size", size}});
        result["fragments"].push_back({{"size", size}, {"type", type}});
        result["regions"].push_back({{"type", type}, {"size", size}});
    };

    addCategory("INSTANCE_OBJECT", categoryTotals[0]);
    addCategory("OBJECT_ARRAY", categoryTotals[1]);
    addCategory("STRUCT_ARRAY", categoryTotals[2]);
    addCategory("PRIMITIVE_ARRAY", categoryTotals[3]);
    addCategory("PINNED_OBJECT", pinnedTotal);
    addCategory("LARGE_OBJECT", largeTotal);
    addCategory("UNMOVABLE_OBJECT", unmovableTotal);
    addCategory("FREE_SPACE", freeSpace);

    return result.dump();
}

std::string HttpHandlers::handleFragmentSummary(const HttpContext& ctx)
{
    LOG_DEBUG("Handling /api/fragment/summary");

    uint64_t instanceTotal = 0;
    uint64_t objectArrayTotal = 0;
    uint64_t structArrayTotal = 0;
    uint64_t primitiveTotal = 0;
    uint64_t pinnedTotal = 0;
    uint64_t largeTotal = 0;
    uint64_t unmovableTotal = 0;

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
                    unmovableTotal += obj.size;
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
    j["unmovable_object_total"] = unmovableTotal;
    j["top10"] = json::array();
    return j.dump();
}

std::string HttpHandlers::handleFragmentDistribution(const HttpContext& ctx)
{
    LOG_DEBUG("Handling /api/fragment/distribution");

    if (!ctx.objects || ctx.objects->empty() || !ctx.snapshotInfo) {
        json j;
        j["address_range_start"] = "0x0";
        j["address_range_end"] = "0x0";
        j["total_address_range"] = 0;
        j["rows"] = json::array();
        return j.dump();
    }

    uint64_t rangeStart = ctx.snapshotInfo->address_range_start;
    uint64_t rangeEnd = ctx.snapshotInfo->address_range_end;
    uint64_t totalRange = (rangeEnd > rangeStart) ? (rangeEnd - rangeStart) : K_MIN_HEAP_SIZE;

    // Use object_id as address (HPROF object IDs are heap addresses).
    // Filter out objects with object_id=0 (invalid) and size=0,
    // then sort by object_id (address).
    std::vector<const HeapObject*> sorted;
    sorted.reserve(ctx.objects->size());
    for (const auto& obj : *ctx.objects) {
        if (obj.object_id > 0 && obj.size > 0) {
            sorted.push_back(&obj);
        }
    }
    if (sorted.empty()) {
        return buildEmptyFallback(rangeStart, rangeEnd, totalRange).dump();
    }
    std::sort(sorted.begin(), sorted.end(),
        [](const HeapObject* a, const HeapObject* b) { return a->object_id < b->object_id; });

    // Main path: bucket -> merge into segments -> group into rows -> JSON.
    auto buckets = buildBuckets(sorted, rangeStart, rangeEnd, totalRange);
    auto segments = mergeSegments(buckets, rangeStart, rangeEnd, totalRange);
    auto rows = groupIntoRows(segments, rangeStart, rangeEnd, totalRange);
    return buildDistributionJson(rangeStart, rangeEnd, totalRange, rows).dump();
}

} // namespace cjprof
