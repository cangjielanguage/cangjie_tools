// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE__CJPROF_H
#define CANGJIE__CJPROF_H

#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <iostream>

namespace Cjprof {

/**
 * Overview information of the heap memory snapshot
 * id: hash id of full filename
 * fileSize: size of the heap memory snapshot file
 * filePath: full file name includes the absolute path
 */
class HeapSnapshot {
public:
    uint64_t id;
    uint64_t fileSize;
    std::string filePath;
};

class InstanceNode {
public:
    std::string className;
    uint32_t distance;
    uint32_t retainedSize;
    uint32_t shallowSize;
    double shallowSizePercent;
    double retainedSizePercent;
    uint32_t totalSize;
    std::vector<InstanceNode> children;
    std::vector<InstanceNode> retainerNodes;
    uint64_t id;
    uint32_t nodeIndex;
    std::string type;
    std::string rootType;
    uint32_t childrenCount;
    uint32_t retainerCount;
    uint32_t startPosition;
    uint32_t endPosition;
    uint32_t arrayLength;
    InstanceNode() = default;
    InstanceNode(const InstanceNode& other) = default;
    bool operator<(const InstanceNode& other) const {
        return false;
    }
};

class InstanceDiffNode : public InstanceNode {
public:
    uint32_t addedCount;
    uint32_t removedCount;
    int64_t countDelta;
    uint32_t addedSize;
    uint32_t removedSize;
    int64_t sizeDelta;
    bool added;
    InstanceDiffNode() = default;
    InstanceDiffNode(
        const InstanceNode& baseNode,
        uint32_t _addedCount,
        uint32_t _removedCount,
        uint32_t _addedSize,
        uint32_t _removedSize,
        bool _added)
        : InstanceNode(baseNode),
          addedCount(_addedCount),
          removedCount(_removedCount),
          countDelta(static_cast<int64_t>(_addedCount) - static_cast<int64_t>(_removedCount)),
          addedSize(_addedSize),
          removedSize(_removedSize),
          sizeDelta(static_cast<int64_t>(_addedSize) - static_cast<int64_t>(_removedSize)),
          added(_added)
          {}
};

class ConstructorNode {
public:
    std::string className;
    uint32_t totalSize;
    uint64_t id;
    uint32_t childrenCount;
    uint32_t distance;
    uint32_t shallowSize;
    uint32_t retainedSize;
    double shallowSizePercent;
    double retainedSizePercent;
    double totalInstanceCountPercent;
    std::vector<InstanceNode> children;
    uint32_t startPosition;
    uint32_t endPosition;
    ConstructorNode() = default;
    ConstructorNode(const ConstructorNode& other) = default;
};

class ConstructorDiffNode : public ConstructorNode {
public:
    uint32_t addedCount;
    uint32_t removedCount;
    int64_t countDelta;
    uint32_t addedSize;
    uint32_t removedSize;
    int64_t sizeDelta;
    uint32_t baseTotalSize;
    uint32_t targetTotalSize;
    ConstructorDiffNode() = default;
    ConstructorDiffNode(
        const ConstructorNode& baseNode,
        uint32_t _addedCount,
        uint32_t _removedCount,
        uint32_t _addedSize,
        uint32_t _removedSize,
        uint32_t _baseTotalSize,
        uint32_t _targetTotalSize)
        : ConstructorNode(baseNode),
          addedCount(_addedCount),
          removedCount(_removedCount),
          countDelta(static_cast<int64_t>(_addedCount) - static_cast<int64_t>(_removedCount)),
          addedSize(_addedSize),
          removedSize(_removedSize),
          sizeDelta(static_cast<int64_t>(_addedSize) - static_cast<int64_t>(_removedSize)),
          baseTotalSize(_baseTotalSize),
          targetTotalSize(_targetTotalSize)
          {}
};

/**
 * Information of the thread
 * name: Cangjie thread name
 * frames: The set of stack frames for this thread
 * id: Thread id
 */
class ThreadInfo {
public:
    /**
     * Information of the stack frame
     * funcName: Function name corresponding to the stack frame
     * fileName: Source file name corresponding to the stack frame
     * line: Source code line number corresponding to the stack frame
     * locals: The set of local node objects corresponding to the stack frame
     * id: Frame id
     */
    struct Frame {
        std::string funcName;
        std::string fileName;
        int line;
        std::vector<InstanceNode> locals;
        uint64_t id;
    };

    std::string name;
    std::vector<Frame> frames;
    uint64_t id;
};

/**
 * Parse all heap memory snapshot files
 * @param filePaths file path of memory snapshots
 * @return true if all memory snapshot files are parsed successfully, otherwise false
 */
bool ParseHeapSnapshotFiles(std::vector<std::string>& filePaths);

/**
 * Clean heap memory snapshots by snapshot IDs
 */
void CleanHeapSnapshotFiles(std::vector<uint64_t> ids);

/**
 * Query overview informations of all memory snapshots
 */
std::vector<HeapSnapshot> QueryAllHeapSnapshot();

/**
 * Get the corresponding memory snapshot ID by the file path
 * @param filePath file path of memory snapshot
 * @return snapshot ID
 */
uint64_t GetSnapshotIDByFilePath(std::string filePath);

/**
 * Get all constructor nodes of the memory snapshot by memory snapshot ID
 * @param id snapshot ID
 * @return all constructor nodes of the memory snapshot
 */
std::vector<ConstructorNode> GetConstructorNodesBySnapshotID(uint64_t id);

/**
 * Get the constructor nodes by root node types
 * @param id snapshot ID
 * @param rootTypes root node types, 0 for -, 1 for global, 2 for local, 3 for unknown
 * @return ConstructorNodes
 */
std::vector<ConstructorNode> GetRootNodesBySnapshotID(uint64_t id, std::set<uint8_t> rootTypes);

/**
 * Get the constructor diff nodes between the two memory snapshots by root node types
 * @param baseSnapshotId base snapshot ID for comparison
 * @param targetSnapshotId target snapshot ID for comparison
 * @param rootTypes root node types
 * @return ConstructorDiffNodes
 */
std::vector<ConstructorDiffNode> GetRootDiffNodesBySnapshotID(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, std::set<uint8_t> rootTypes);

/**
 * Expand a constructor node
 * @param snapshotId snapshot ID
 * @param nodeId constructor node ID
 * @param startIndex pagination start position
 * @param length page length
 * @return Information after expanding the constructor node
 */
ConstructorNode ExpandConstructorNode(
    uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);

/**
 * Expand the constructor node to show the differences between the two memory snapshots
 * @param baseSnapshotId base snapshot ID for comparison
 * @param targetSnapshotId target snapshot ID for comparison
 * @param nodeId constructor node ID
 * @param startIndex pagination start position
 * @param length page length
 * @return Information after expanding the constructor node
 */
ConstructorDiffNode ExpandConstructorDiffNode(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);

/**
 * Expand an instance node
 * @param snapshotId snapshot ID
 * @param nodeId instance node ID
 * @param startIndex pagination start position
 * @param length page length
 * @return Information after expanding the instance node
 */
InstanceNode ExpandInstanceNode(
    uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);

/**
 * Expand the instance node to show the differences between the two memory snapshots
 * @param baseSnapshotId base snapshot ID for comparison
 * @param targetSnapshotId target snapshot ID for comparison
 * @param nodeId instance node ID
 * @param startIndex pagination start position
 * @param length page length
 * @return Information after expanding the instance node
 */
InstanceDiffNode ExpandInstanceDiffNode(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);

/**
 * Expand the field node or reference node of an instance node
 * @param snapshotId snapshot ID
 * @param nodeId instance node ID
 * @param isReference true indicates to expand the references of the node,
 *      and false indicates to expand the fields of the node
 * @param startIndex pagination start position
 * @param length page length
 * @return InstanceNode information after expanding the instance detail node
 */
InstanceNode ExpandDetailNode(
    uint64_t snapshotId, uint64_t nodeId, bool isReference, uint32_t startIndex, uint32_t length);

/**
 * Expand the field node or reference node of a difference instance node between two memory snapshots
 * @param baseSnapshotId base snapshot ID for comparison
 * @param targetSnapshotId target snapshot ID for comparison
 * @param nodeId instance node ID
 * @param isReference true indicates to expand the references of the node,
 *      and false indicates to expand the fields of the node
 * @param startIndex pagination start position
 * @param length page length
 * @return InstanceDiffNode information after expanding the instance detail node
 */
InstanceDiffNode ExpandDetailDiffNode(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId,
    bool isReference, uint32_t startIndex, uint32_t length);

/**
 * Query the comparison result between the two memory snapshots
 * @param baseId base snapshot ID for comparison
 * @param targetId target snapshot ID for comparison
 * @return the difference information of all ConstructorNodes after comparing the two memory snapshots
 */
std::vector<ConstructorDiffNode> QuerySnapshotComparison(uint64_t baseId, uint64_t targetId);

/**
 * Get the set of paths from a node to the root node and sort these paths in descending order by retainedSize
 * @param snapshotId snapshot ID
 * @param nodeId instance ID
 * @param pathNum number of paths from a node to root node, The value -1 indicates all
 * @return the set of paths
 */
std::vector<std::vector<InstanceNode>> GetNodeRootpaths(uint64_t snapshotId, uint64_t nodeId, int32_t pathNum);

/**
 * Get information about all threads of the memory snapshot by the snapshot ID
 * @param snapshotId snapshot ID
 * @return all threads information of the memory snapshot
 */
std::vector<ThreadInfo> GetThreadInfos(uint64_t snapshotId);

/**
 * Query the number of nodes in a memory snapshot by keyword
 * @param keyword keyword
 * @param isIgnoreCase ignore case
 * @param snapshotId snapshot ID
 * @return the number of nodes
 */
uint32_t QuerySnapshotCountOfResults(
    std::string keyword, bool isIgnoreCase, uint64_t snapshotId);

/**
 * Query the constructor node information corresponding to the index
 * @param keyword keyword
 * @param isIgnoreCase ignore case
 * @param snapshotId snapshot ID
 * @param length length of ConstructorNode.children
 * @param index constructor node index, the value range is (1, QuerySnapshotCountOfResults())
 * @return ConstructorNode
 */
ConstructorNode QuerySnapshotNodeByIndex(
    std::string keyword, bool isIgnoreCase, uint64_t snapshotId, uint32_t length, uint32_t index);

/**
 * Query the number of the difference nodes between two memory snapshots by keyword
 * @param keyword keyword
 * @param isIgnoreCase ignore case
 * @param baseId base snapshot ID for comparison
 * @param targetId target snapshot ID for comparison
 * @return the number of nodes
 */
uint32_t QueryComparisonCountOfResults(
    std::string keyword, bool isIgnoreCase, uint64_t baseId, uint64_t targetId);

/**
 * Query the constructor diff node information corresponding to the index
 * @param keyword keyword
 * @param isIgnoreCase ignore case
 * @param baseId base snapshot ID for comparison
 * @param targetId target snapshot ID for comparison
 * @param length length of ConstructorNode.children
 * @param index constructor diff node index, the value range is (1, QueryComparisonCountOfResults())
 * @return ConstructorDiffNode
 */
ConstructorDiffNode QueryComparisonNodeByIndex(
    std::string keyword, bool isIgnoreCase, uint64_t baseId, uint64_t targetId, uint32_t length, uint32_t index);

/**
 * Result of computing a dominance tree over a directed graph.
 */
struct DominanceTreeResult {
    // dom[nodeOrdinal] = parentOrdinal. nodeOrdinal ranges in [0..n] where 0 is the virtual entry node.
    std::vector<size_t> dom;
    // domTree[parent].push_back(child)
    std::vector<std::vector<size_t>> domTree;
    size_t noEntry;
};

/**
 * Compute the dominance tree using Cooper's algorithm.
 *
 * @param n       Number of original nodes (excluding the virtual entry).
 * @param succs   succs[i] = list of nodes reachable from node i via outgoing edges.
 * @param preds   preds[i] = list of nodes that have an edge to node i.
 * @param gcRoots List of root node indices (will be connected to the virtual entry node 0).
 * @return DominanceTreeResult containing dom and domTree.
 */
DominanceTreeResult ComputeDominanceTree(
    size_t n,
    const std::vector<std::vector<size_t>>& succs,
    const std::vector<std::vector<size_t>>& preds,
    const std::vector<size_t>& gcRoots);

} // namespace Cjprof
#endif // CANGJIE__CJPROF_H