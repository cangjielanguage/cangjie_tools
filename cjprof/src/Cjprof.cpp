// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Cjprof.h"
#include <algorithm>
#include "Analyzer/HeapAnalyzer.h"
#include <utility>
#include <functional>
#include <list>
#include <queue>
#ifdef USE_CXX17_FEATURES
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace Cjprof {

static size_t intersect(const std::vector<size_t>& idom, size_t p1, size_t p2)
{
    while (p2 != p1) {
        while (p2 < p1) {
            p2 = idom[p2];
        }
        while (p1 < p2) {
            p1 = idom[p1];
        }
    }
    return p1;
}

static void dfs(size_t node,
                const std::vector<std::vector<size_t>>& graph,
                std::vector<size_t>& visited,
                std::vector<size_t>& reversePostOrder) {
    visited[node] = 1;
    for (size_t succ : graph[node]) {
        if (!visited[succ]) {
            dfs(succ, graph, visited, reversePostOrder);
        }
    }
    reversePostOrder.push_back(node);
}

struct PostOrderMaps {
    const std::unordered_map<size_t, size_t>& postOrderIndex2Index;
    const std::unordered_map<size_t, size_t>& index2PostOrderIndex;
};

static bool tryUpdateNewDom(
    const std::vector<size_t>& idom,
    size_t noEntry,
    size_t entryPostOrderedIndex,
    size_t retainerPostOrderIndex,
    size_t& newDom)
{
    if (idom[retainerPostOrderIndex] == noEntry) {
        return false;
    }
    if (newDom == noEntry) {
        newDom = retainerPostOrderIndex;
    } else {
        newDom = intersect(idom, newDom, retainerPostOrderIndex);
    }
    return newDom == entryPostOrderedIndex;
}

static std::vector<size_t> computeIdom(
    size_t entryPostOrderedIndex,
    size_t noEntry,
    const std::vector<std::vector<size_t>>& predList,
    const std::vector<std::vector<size_t>>& graph,
    const PostOrderMaps& maps)
{
    std::vector<size_t> idom(maps.postOrderIndex2Index.size(), noEntry);
    idom[entryPostOrderedIndex] = entryPostOrderedIndex;

    std::vector<bool> affected(maps.postOrderIndex2Index.size(), false);
    for (auto succ : graph[0]) {
        auto it = maps.index2PostOrderIndex.find(succ);
        if (it != maps.index2PostOrderIndex.end()) {
            affected[it->second] = true;
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto postOrderIndex = entryPostOrderedIndex; postOrderIndex-- > 0;) {
            if (!affected[postOrderIndex]) {
                continue;
            }
            affected[postOrderIndex] = false;
            if (idom[postOrderIndex] == entryPostOrderedIndex) {
                continue;
            }
            auto newDom = noEntry;
            for (auto p : predList[maps.postOrderIndex2Index.at(postOrderIndex)]) {
                auto it = maps.index2PostOrderIndex.find(p);
                if (it == maps.index2PostOrderIndex.end()) {
                    continue; // skip unreachable predecessor
                }
                auto retainerPostOrderIndex = it->second;
                if (tryUpdateNewDom(idom, noEntry, entryPostOrderedIndex,
                                    retainerPostOrderIndex, newDom)) {
                    break;
                }
            }
            if (newDom != noEntry && idom[postOrderIndex] != newDom) {
                idom[postOrderIndex] = newDom;
                changed = true;
                for (auto succ : graph[maps.postOrderIndex2Index.at(postOrderIndex)]) {
                    auto it = maps.index2PostOrderIndex.find(succ);
                    if (it != maps.index2PostOrderIndex.end()) {
                        affected[it->second] = true;
                    }
                }
            }
        }
    }
    return idom;
}

DominanceTreeResult ComputeDominanceTree(
    size_t n,
    const std::vector<std::vector<size_t>>& succs,
    const std::vector<std::vector<size_t>>& preds,
    const std::vector<size_t>& gcRoots)
{
    size_t entry = 0;
    size_t totalNodes = n + 1;
    std::vector<std::vector<size_t>> graph(totalNodes);
    std::vector<std::vector<size_t>> predList(totalNodes);

    for (size_t i = 0; i < n; ++i) {
        size_t src = i + 1;
        for (size_t succ : succs[i]) {
            size_t dst = succ + 1;
            graph[src].push_back(dst);
            predList[dst].push_back(src);
        }
    }

    for (size_t root : gcRoots) {
        size_t rootIdx = root + 1;
        graph[entry].push_back(rootIdx);
        predList[rootIdx].push_back(entry);
    }

    std::vector<size_t> visited(totalNodes, 0);
    std::vector<size_t> postOrder;
    dfs(entry, graph, visited, postOrder);

    std::unordered_map<size_t, size_t> postOrderIndex2Index;
    std::unordered_map<size_t, size_t> index2PostOrderIndex;
    for (size_t i = 0; i < postOrder.size(); ++i) {
        auto idx = postOrder[i];
        postOrderIndex2Index[i] = idx;
        index2PostOrderIndex[idx] = i;
    }

    // Cooper et al. Algorithm
    auto entryPostOrderedIndex = postOrder.size() - 1;
    auto noEntry = postOrder.size();
    PostOrderMaps maps{postOrderIndex2Index, index2PostOrderIndex};
    auto idom = computeIdom(entryPostOrderedIndex, noEntry, predList, graph, maps);

    std::vector<size_t> dom(totalNodes, noEntry);
    for (size_t postOrderIndex = 0; postOrderIndex < postOrder.size(); ++postOrderIndex) {
        auto idx = postOrderIndex2Index[postOrderIndex];
        dom[idx] = postOrderIndex2Index[idom[postOrderIndex]];
    }

    std::vector<std::vector<size_t>> domTree(totalNodes);
    for (size_t v = 1; v <= n; ++v) {
        if (dom[v] != noEntry) {
            domTree[dom[v]].push_back(v);
        }
    }

    return {dom, domTree, noEntry};
}

class RawHeapSnapshotData {
public:
    struct InsNode {
        uint64_t id;
        uint32_t distance;                  // shortest distance to root node
        uint32_t shallowSize;
        uint32_t retainedSize;
        uint32_t nameIndex;                 // index in rhs.strings
        uint32_t nodeIndex;                 // index in insNodes
        std::vector<uint32_t> children;     // child index in insNodes
        std::vector<uint32_t> references;   // parent index in insNodes
        bool isRoot;
        bool isPinned = false;
    };
    struct ConNode {
        uint64_t id;
        uint32_t distance;                  // shortest distance in the child node distances
        uint32_t shallowSize;
        uint32_t retainedSize;
        uint32_t nameIndex;                 // index in rhs.strings
        std::vector<uint32_t> children;     // child index in insNodes
    };
    RawHeapSnapshot rhs;
    std::vector<InsNode> insNodes;
    std::vector<ConNode> conNodes;
    std::unordered_map<uint64_t, uint64_t> insNodeIdToIndex; // id -> index
    uint32_t totalSize;
    uint32_t totalInstanceCount;

    void CalculateRetainedSize()
    {
        size_t n = insNodes.size();

        std::vector<std::vector<size_t>> preds(n);
        std::vector<std::vector<size_t>> succs(n);

        for (size_t i = 0; i < n; ++i) {
            for (uint32_t childIdx : insNodes[i].children) {
                if (childIdx >= 0 && childIdx < n) {
                    succs[i].push_back(childIdx);
                    preds[childIdx].push_back(i);
                }
            }
        }

        std::vector<size_t> gcRoots;
        for (size_t i = 0; i < n; ++i) {
            if (preds[i].empty() || insNodes[i].isRoot || insNodes[i].isPinned) {
                gcRoots.push_back(i);
            }
        }

        if (gcRoots.empty()) {
            return;
        }

        auto result = ComputeDominanceTree(n, succs, preds, gcRoots);
        const auto& dom = result.dom;
        const auto& domTree = result.domTree;

        for (size_t v = 1; v <= n; ++v) {
            if (dom[v] == 0) {
                computeRetainedSize(v, insNodes, domTree);
            }
        }

        for (size_t i = 0; i < conNodes.size(); ++i) {
            uint32_t totalShallowSize = 0;
            uint32_t totalRetainedSize = 0;

            for (uint32_t child : conNodes[i].children) {
                totalShallowSize += insNodes[child].shallowSize;
                totalRetainedSize += insNodes[child].retainedSize;
            }

            conNodes[i].shallowSize = totalShallowSize;
            conNodes[i].retainedSize = totalRetainedSize;
        }
    }

    static void dfs(size_t node,
                    const std::vector<std::vector<size_t>>& graph,
                    std::vector<size_t>& visited,
                    std::vector<size_t>& reversePostOrder) {
        visited[node] = 1;
        for (size_t succ : graph[node]) {
            if (!visited[succ]) {
                dfs(succ, graph, visited, reversePostOrder);
            }
        }
        reversePostOrder.push_back(node);
    }

    static uint32_t computeRetainedSize(size_t node,
                                        std::vector<InsNode>& nodes,
                                        const std::vector<std::vector<size_t>>& domTree) {
        size_t originalIdx = node - 1;
        uint32_t size = nodes[originalIdx].shallowSize;
        for (size_t child : domTree[node]) {
            size += computeRetainedSize(child, nodes, domTree);
        }
        nodes[originalIdx].retainedSize = size;
        return size;
    }

    // DFS Recursively Searches for All Paths (index List)
    void DfsFindAllPaths(uint32_t currNodeIdx,
                        std::vector<uint32_t>& currentPath,
                        std::unordered_set<uint32_t>& visitedNodes,
                        std::vector<std::vector<uint32_t>>& allPaths) {
        if (visitedNodes.count(currNodeIdx)) {
            return;
        }

        visitedNodes.insert(currNodeIdx);
        currentPath.push_back(currNodeIdx);

        const auto& currNode = this->insNodes[currNodeIdx];
        if (currNode.isRoot) {
            allPaths.push_back(currentPath);
        }
        for (uint32_t parentIdx : currNode.references) {
            DfsFindAllPaths(parentIdx, currentPath, visitedNodes, allPaths);
        }

        currentPath.pop_back();
        visitedNodes.erase(currNodeIdx);
    }

    // Get all paths from a specified node to the root node.
    std::vector<std::vector<uint32_t>> GetAllPathsToRoot(uint32_t nodeIdx)
    {
        std::vector<std::vector<uint32_t>> allPaths;
        std::vector<uint32_t> currentPath;
        std::unordered_set<uint32_t> visitedNodes;
        DfsFindAllPaths(nodeIdx, currentPath, visitedNodes, allPaths);
        return allPaths;
    }

    void CalculateDistance()
    {
        std::queue<uint32_t> bfsQueue;
        for (auto& insNode : this->insNodes) {
            insNode.distance = 0;
            if (insNode.isRoot) {
                insNode.distance = 1;
                bfsQueue.push(insNode.nodeIndex);
            }
        }

        while (!bfsQueue.empty()) {
            uint32_t currIdx = bfsQueue.front();
            bfsQueue.pop();
            auto& currNode = this->insNodes[currIdx];
            for (uint32_t childIdx : currNode.children) {
                auto& childNode = this->insNodes[childIdx];
                if (childNode.distance == 0) {
                    childNode.distance = currNode.distance + 1;
                    bfsQueue.push(childIdx);
                }
            }
        }

        for (auto& conNode : this->conNodes) {
            for (auto child : conNode.children) {
                auto& inNode = this->insNodes[child];
                if (conNode.distance == 0) {
                    conNode.distance = inNode.distance;
                    continue;
                }
                if (inNode.distance < conNode.distance) {
                    conNode.distance = inNode.distance;
                }
            }
        }
    }

    void CalculateRawHeapSnapshot()
    {
        auto& rhs = this->rhs;
        this->totalInstanceCount = rhs.nodeCount;

        // name index in strings, node index in nodes of RawHeapSnapshot.
        std::unordered_map<uint32_t, std::vector<uint32_t>> nodeIndexsMap;
        // node index in nodes, node reference index in nodes of RawHeapSnapshot.
        std::unordered_map<uint32_t, std::vector<uint32_t>> nodeRefIndexsMap;

        uint32_t nodeIndex = 0;
        uint32_t edgeIndex = 0;
        for (auto& node : rhs.nodes) {
            nodeIndexsMap[node.nameIndex].emplace_back(node.nodeIndex);
            RawHeapSnapshotData::InsNode insNode = {};
            insNode.nodeIndex = nodeIndex;
            insNode.id = node.id;
            insNode.nameIndex = node.nameIndex;
            insNode.shallowSize = node.selfSize;
            insNode.isRoot = node.IsRoot();
            insNode.isPinned = node.isPinned;
            for (uint32_t i = 0; i < node.edgeCount; i++) {
                if (edgeIndex + i >= rhs.edges.size()) break;
                auto toNodeIndex = rhs.edges[edgeIndex + i].toNode;
                if (toNodeIndex >= rhs.nodes.size()) {
                    continue;
                }
                insNode.children.emplace_back(toNodeIndex);
                nodeRefIndexsMap[toNodeIndex].emplace_back(node.nodeIndex);
            }
            edgeIndex += node.edgeCount;
            if (this->totalSize >= UINT32_MAX - node.selfSize) {
                this->totalSize = UINT32_MAX;
            } else {
                this->totalSize += node.selfSize;
            }
            this->insNodes.emplace_back(insNode);
            this->insNodeIdToIndex.emplace(insNode.id, nodeIndex);
            nodeIndex++;
        }

        for (auto& [index, refIndexs]: nodeRefIndexsMap) {
            this->insNodes[index].references = refIndexs;
        }

        for (auto& [nameIndex, nodeIndexs]: nodeIndexsMap) {
            RawHeapSnapshotData::ConNode conNode = {};
            conNode.id = std::hash<std::string>()(rhs.strings[nameIndex]);
            conNode.nameIndex = nameIndex;
            conNode.children = nodeIndexs;
            this->conNodes.emplace_back(conNode);
        }
        CalculateRetainedSize();
        CalculateDistance();
    }

    InstanceNode CreateInstanceNode(uint32_t index)
    {
        auto insNode = InstanceNode();
        auto& node = this->insNodes[index];
        insNode.className = this->rhs.strings[node.nameIndex];
        insNode.id = node.id;
        insNode.nodeIndex = node.nodeIndex;
        insNode.shallowSize = node.shallowSize;
        insNode.retainedSize = node.retainedSize;
        insNode.totalSize = this->totalSize;
        insNode.shallowSizePercent = (this->totalSize > 0) ? 100.0 * insNode.shallowSize / this->totalSize : 0.0;
        insNode.retainedSizePercent = (this->totalSize > 0) ? 100.0 * insNode.retainedSize / this->totalSize : 0.0;
        insNode.distance = node.distance;
        insNode.type = this->rhs.nodes[index].TypeString();
        insNode.rootType = this->rhs.nodes[index].RootString();
        insNode.arrayLength = this->rhs.nodes[index].arrayLen;
        insNode.childrenCount = node.children.size();
        insNode.retainerCount = node.references.size();
        return insNode;
    }

    ConstructorNode CreateConstructorNode(ConNode& con)
    {
        auto conNode = ConstructorNode();
        conNode.className = this->rhs.strings[con.nameIndex];
        conNode.childrenCount = con.children.size();
        conNode.id = con.id;
        conNode.distance = con.distance;
        conNode.shallowSize = con.shallowSize;
        conNode.retainedSize = con.retainedSize;
        conNode.totalSize = this->totalSize;
        conNode.shallowSizePercent = (this->totalSize > 0) ? 100.0 * conNode.shallowSize / this->totalSize : 0.0;
        conNode.retainedSizePercent = (this->totalSize > 0) ? 100.0 * conNode.retainedSize / this->totalSize : 0.0;
        conNode.totalInstanceCountPercent =
            (this->totalInstanceCount > 0) ? 100.0 * conNode.childrenCount / this->totalInstanceCount : 0.0;
        return conNode;
    }

    std::vector<ConstructorNode> GetAllConstructorNodes() {
        std::vector<ConstructorNode> conNodes;
        for (auto& con: this->conNodes) {
            conNodes.emplace_back(CreateConstructorNode(con));
        }
        return conNodes;
    }

    std::vector<std::vector<InstanceNode>> GetNodeRootpaths(uint64_t nodeId, int32_t pathNum) {
        if (pathNum < -1) {
            return {};
        }
        auto it = insNodeIdToIndex.find(nodeId);
        if (it == insNodeIdToIndex.end()) {
            return {};
        }
        auto pathList = GetAllPathsToRoot(it->second);
        // sort pathList in descending order by retainedSize
        std::sort(pathList.begin(), pathList.end(),
            [this](const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
                if (a.empty() || b.empty()) {
                    return false;
                }
                const auto& aNode = insNodes[a.back()];
                const auto& bNode = insNodes[b.back()];
                if (aNode.retainedSize != bNode.retainedSize) {
                    return aNode.retainedSize > bNode.retainedSize;
                }
                if (a.size() != b.size()) {
                    return a.size() < b.size();
                }
                return false;
            }
        );

        // if pathNum is -1, get all path lists.
        unsigned long num = pathList.size();
        if (pathNum >= 0) {
            unsigned long unsigned_pathNum = static_cast<unsigned long>(pathNum);
            if (unsigned_pathNum < num) {
                num = unsigned_pathNum;
            }
        }
        if (num <= 0) {
            return {};
        }

        // Insert the first n elements.
        std::vector<std::vector<InstanceNode>> result;
        result.reserve(num);
        for (unsigned long i = 0; i < num; ++i) {
            const auto& nodePath = pathList[i];
            std::vector<InstanceNode> insNodePath;
            insNodePath.reserve(nodePath.size());
            for (uint32_t index : nodePath) {
                insNodePath.emplace_back(CreateInstanceNode(index));
            }
            result.push_back(std::move(insNodePath));
        }
        return result;
    }

    std::vector<ConstructorNode> GetRootNodes(std::set<uint8_t>& rootTypes) {
        std::vector<ConstructorNode> conNodes;
        for (auto& con: this->conNodes) {
            auto conNode = CreateConstructorNode(con);
            for (auto& index : con.children) {
                auto it = std::find(rootTypes.begin(), rootTypes.end(), this->rhs.nodes[index].rootType);
                if (it == rootTypes.end()) {
                    continue;
                }
                auto insNode = CreateInstanceNode(index);
                conNode.children.emplace_back(insNode);
            }
            conNode.childrenCount = conNode.children.size();
            if (conNode.childrenCount <= 0) {
                continue;
            }
            conNodes.emplace_back(conNode);
        }
        return conNodes;
    }

    ConstructorNode GetConstructorNode(uint64_t nodeId, uint32_t startIndex, uint32_t length)
    {
        for (auto& con: this->conNodes) {
            if (con.id != nodeId) {
                continue;
            }
            auto conNode = CreateConstructorNode(con);
            if (conNode.childrenCount <=0 || length <= 0) {
                return conNode;
            }
            conNode.startPosition = startIndex;
            conNode.endPosition = startIndex + length - 1;
            auto endIndex = conNode.endPosition;
            auto childrenCount = con.children.size();
            if (endIndex >= childrenCount) {
                endIndex = childrenCount - 1;
            }
            for (uint32_t i = startIndex; i <= endIndex; i++) {
                auto index = con.children[i];
                auto insNode = CreateInstanceNode(index);
                conNode.children.emplace_back(insNode);
            }
            return conNode;
        }
        return ConstructorNode();
    }

    std::vector<ThreadInfo> GetThreadInfos() {
        std::vector<ThreadInfo> threads;
        for (auto& th : rhs.threads) {
            ThreadInfo thread;
            thread.name = th.name;
            thread.id = std::hash<std::string>()(thread.name);
            for (auto& fr : th.frames) {
                ThreadInfo::Frame frame;
                frame.funcName = rhs.strings[fr.funcName];
                frame.id = std::hash<std::string>()(frame.funcName);
                frame.fileName = (fr.fileName < rhs.strings.size()) ? rhs.strings[fr.fileName] : "";
                frame.line = fr.line;
                for (auto index : fr.locals) {
                    frame.locals.emplace_back(CreateInstanceNode(index));
                }
                thread.frames.emplace_back(frame);
            }
            threads.emplace_back(thread);
        }
        return threads;
    }
};

// heapSnapshotID, RawHeapSnapshotData
std::unordered_map<uint64_t, RawHeapSnapshotData> RawHeapSnapshotDatas;

struct PairHash {
    size_t operator()(const std::pair<uint64_t, uint64_t>& key) const {
        size_t h1 = std::hash<uint64_t>{}(key.first);
        size_t h2 = std::hash<uint64_t>{}(key.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

class ConstructorDiffData {
public:
    uint32_t addedCount;
    uint32_t removedCount;
    uint32_t addedSize;
    uint32_t removedSize;
};

using AllConstructorDiffData = std::unordered_map<std::string, ConstructorDiffData>;

// baseSnapshotId, targetSnapshotId -> AllConstructorDiffData
std::unordered_map<std::pair<uint64_t, uint64_t>, AllConstructorDiffData, PairHash> SnapshotDiffs;

// baseSnapshotId, targetSnapshotId -> std::vector<ConstructorDiffNode>
std::unordered_map<std::pair<uint64_t, uint64_t>, std::vector<ConstructorDiffNode>, PairHash> SnapshotComparison;

bool ParseHeapSnapshotFile(std::string& filePath)
{
    if (!fs::exists(filePath)) {
        return false;
    }
    std::string canonPath = fs::canonical(filePath).generic_string();
    auto analyzer = HeapAnalyzer();
    if (!analyzer.SetData(canonPath)) {
        return false;
    }
    if (!analyzer.Analyze(false)) {
        return false;
    }

    // HeapAnalyzer -> RawHeapSnapshot
    RawHeapSnapshotData rhsd = {};
    rhsd.rhs = analyzer.GetRawHeapSnapshot();
    if (rhsd.rhs.nodeCount == 0) {
        return false;
    }
    auto hashid = std::hash<std::string>()(canonPath); // fullname
    rhsd.rhs.hashid = hashid;
    rhsd.rhs.filePath = canonPath;

    rhsd.CalculateRawHeapSnapshot();
    RawHeapSnapshotDatas[hashid] = rhsd;
    return true;
}

bool ParseHeapSnapshotFiles(std::vector<std::string>& filePaths)
{
    for (auto& fp : filePaths) {
        if (!ParseHeapSnapshotFile(fp)) {
            return false;
        }
    }
    return true;
}

void CleanHeapSnapshotFiles(std::vector<uint64_t> ids)
{
    SnapshotComparison.clear();
    SnapshotDiffs.clear();
    if (ids.empty()) {
        return;
    }

    std::unordered_set<uint64_t> idSet(ids.begin(), ids.end());
    for (auto it = RawHeapSnapshotDatas.begin(); it != RawHeapSnapshotDatas.end(); ) {
        if (idSet.count(it->first)) {
            // erase returns the next valid iterator.
            it = RawHeapSnapshotDatas.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<HeapSnapshot> QueryAllHeapSnapshot()
{
    std::vector<HeapSnapshot> heapSnapshots;
    for (auto& [id, rhsd] : RawHeapSnapshotDatas) {
        heapSnapshots.emplace_back(HeapSnapshot{rhsd.rhs.hashid, rhsd.rhs.fileSize, rhsd.rhs.filePath});
    }
    return heapSnapshots;
}

uint64_t GetSnapshotIDByFilePath(std::string filePath)
{
    if (!fs::exists(filePath)) {
        return 0;
    }
    std::string canonPath = fs::canonical(filePath).generic_string();
    for (auto& [hashid, rhsd] : RawHeapSnapshotDatas) {
        if (canonPath == rhsd.rhs.filePath) {
            return hashid;
        }
    }
    return std::hash<std::string>()(canonPath);
}

std::vector<ConstructorNode> GetConstructorNodesBySnapshotID(uint64_t id)
{
    auto it = RawHeapSnapshotDatas.find(id);
    if (it == RawHeapSnapshotDatas.end()) {
        return {};
    }

    return it->second.GetAllConstructorNodes();
}

std::vector<std::vector<InstanceNode>> GetNodeRootpaths(uint64_t snapshotId, uint64_t nodeId, int32_t pathNum)
{
    auto it = RawHeapSnapshotDatas.find(snapshotId);
    if (it == RawHeapSnapshotDatas.end()) {
        return {};
    }

    return it->second.GetNodeRootpaths(nodeId, pathNum);
}

/**
 * @brief      Checks if a string contains a keyword
 *
 * @param[in]  name             Class name
 * @param[in]  keyword          Keyword
 * @param[in]  isIgnoreCase     Whether to ignore case
 * @return     true if keyword is contained in name; false otherwise
 */
static bool IsKeywordContained(const std::string& name, const std::string& keyword, bool isIgnoreCase)
{
    if (keyword.empty()) return true;
    if (keyword.size() > name.size()) return false;

    if (!isIgnoreCase) {
        return name.find(keyword) != std::string::npos;
    } else {
        auto caseInsensitiveCharCompare = [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
        };

        auto it = std::search(name.begin(), name.end(),
                              keyword.begin(), keyword.end(),
                              caseInsensitiveCharCompare);

        return it != name.end();
    }
}

/**
 * @brief      Query the number of InstanceNodes in a memory snapshot by keyword
 *
 * @param[in]  keyword          Keyword
 * @param[in]  isIgnoreCase     Whether to ignore case
 * @param[in]  snapshotId       ID of snapshot
 * @return     Number of InstanceNodes
 */
uint32_t QuerySnapshotCountOfResults(std::string keyword, bool isIgnoreCase, uint64_t snapshotId)
{
    std::vector<ConstructorNode> constructorNodes = GetConstructorNodesBySnapshotID(snapshotId);

    uint32_t res = 0;
    for (auto& constructorNode : constructorNodes) {
        if (IsKeywordContained(constructorNode.className, keyword, isIgnoreCase)) {
            res += constructorNode.childrenCount;
        }
    }

    return res;
}

/**
 * @brief      Query the number of InstanceDiffNodes in difference between two memory snapshots by keyword
 *
 * @param[in]  keyword          Keyword
 * @param[in]  isIgnoreCase     Whether to ignore case
 * @param[in]  baseId           ID of base snapshot
 * @param[in]  targetId         ID of target snapshot
 * @return     Number of InstanceDiffNodes
 */
uint32_t QueryComparisonCountOfResults(std::string keyword, bool isIgnoreCase, uint64_t baseId, uint64_t targetId)
{
    std::vector<ConstructorDiffNode> constructorDiffNodes = QuerySnapshotComparison(baseId, targetId);

    uint32_t res = 0;
    for (auto& constructorDiffNode : constructorDiffNodes) {
        if (!IsKeywordContained(constructorDiffNode.className, keyword, isIgnoreCase)) {
            continue;
        }
        if (constructorDiffNode.addedCount == 0) {
            res += constructorDiffNode.removedCount;
        } else if (constructorDiffNode.removedCount == 0) {
            res += constructorDiffNode.addedCount;
        } else {
            auto expanded = ExpandConstructorDiffNode(
                baseId, targetId, constructorDiffNode.id, 0,
                constructorDiffNode.addedCount + constructorDiffNode.removedCount);
            res += expanded.childrenCount;
        }
    }

    return res;
}

/**
 * @brief      Query ConstructorNode corresponding to the index in QuerySnapshotCountOfResults
 *
 * @param[in]  keyword          Keyword
 * @param[in]  isIgnoreCase     Whether to ignore case
 * @param[in]  snapshotId       ID of snapshot
 * @param[in]  length           Max length of children list for each ConstructorNode
 * @param[in]  index            Count of InstanceNode
 * @return     ConstructorNode which contains searched InstanceNode
 */
ConstructorNode QuerySnapshotNodeByIndex(std::string keyword, bool isIgnoreCase,
    uint64_t snapshotId, uint32_t length, uint32_t index)
{
    if (index == 0) {
        return ConstructorNode();
    }
    std::vector<ConstructorNode> constructorNodes = GetConstructorNodesBySnapshotID(snapshotId);

    auto curIndex = index - 1;
    for (auto& constructorNode : constructorNodes) {
        if (IsKeywordContained(constructorNode.className, keyword, isIgnoreCase)) {
            if (curIndex < constructorNode.childrenCount) {
                return ExpandConstructorNode(snapshotId, constructorNode.id, curIndex, length);
            }
            curIndex -= constructorNode.childrenCount;
        }
    }

    return ConstructorNode();
}

/**
 * @brief      Query ConstructorDiffNode corresponding to the index in QueryComparisonCountOfResults
 *
 * @param[in]  keyword          Keyword
 * @param[in]  isIgnoreCase     Whether to ignore case
 * @param[in]  baseId           ID of base snapshot
 * @param[in]  targetId         ID of target snapshot
 * @param[in]  length           Max length of children list for each ConstructorDiffNode
 * @param[in]  index            Count of InstanceDiffNode
 * @return     ConstructorDiffNode which contains searched InstanceDiffNode
 */
ConstructorDiffNode QueryComparisonNodeByIndex(std::string keyword, bool isIgnoreCase,
    uint64_t baseId, uint64_t targetId, uint32_t length, uint32_t index)
{
    if (index == 0) {
        return ConstructorDiffNode();
    }
    std::vector<ConstructorDiffNode> constructorDiffNodes = QuerySnapshotComparison(baseId, targetId);

    auto curIndex = index - 1;
    for (auto& constructorDiffNode : constructorDiffNodes) {
        if (!IsKeywordContained(constructorDiffNode.className, keyword, isIgnoreCase)) {
            continue;
        }

        if (constructorDiffNode.addedCount == 0) {
            if (curIndex < constructorDiffNode.removedCount) {
                return ExpandConstructorDiffNode(baseId, targetId, constructorDiffNode.id, curIndex, length);
            }
            curIndex -= constructorDiffNode.removedCount;
        } else if (constructorDiffNode.removedCount == 0) {
            if (curIndex < constructorDiffNode.addedCount) {
                return ExpandConstructorDiffNode(baseId, targetId, constructorDiffNode.id, curIndex, length);
            }
            curIndex -= constructorDiffNode.addedCount;
        } else {
            auto expanded = ExpandConstructorDiffNode(baseId, targetId, constructorDiffNode.id, curIndex, length);
            if (curIndex < expanded.childrenCount) {
                return expanded;
            }
            curIndex -= expanded.childrenCount;
        }
    }

    return ConstructorDiffNode();
}

ConstructorNode ExpandConstructorNode(
    uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length)
{
    auto it = RawHeapSnapshotDatas.find(snapshotId);
    if (it == RawHeapSnapshotDatas.end()) {
        return {};
    }

    return it->second.GetConstructorNode(nodeId, startIndex, length);
}

void resetSnapshotDiffs(uint64_t baseId, uint64_t targetId)
{
    SnapshotDiffs.clear();
    SnapshotDiffs[{baseId, targetId}] = AllConstructorDiffData();
    SnapshotDiffs[{targetId, baseId}] = AllConstructorDiffData();
}

void updateComparisonCache(const uint64_t baseId, const uint64_t targetId,
    const std::vector<ConstructorDiffNode>& constructorDiffNodes)
{
    const auto keyBaseTarget = std::make_pair(baseId, targetId);
    const auto keyTargetBase = std::make_pair(targetId, baseId);
    if (!SnapshotDiffs.count(keyBaseTarget)) {
        resetSnapshotDiffs(baseId, targetId);
    }
    auto& diffBaseTarget = SnapshotDiffs[keyBaseTarget];
    auto& diffTargetBase = SnapshotDiffs[keyTargetBase];
    for (auto &diffNode: constructorDiffNodes) {
        diffBaseTarget[diffNode.className] = {
            diffNode.addedCount, diffNode.removedCount, diffNode.addedSize, diffNode.removedSize};
        diffTargetBase[diffNode.className] = {
            diffNode.removedCount, diffNode.addedCount, diffNode.removedSize, diffNode.addedSize};
    }
    SnapshotComparison.clear();
    SnapshotComparison[keyBaseTarget] = constructorDiffNodes;
}

bool isSameConstructorNode(const uint64_t baseId, const uint64_t targetId,
    const ConstructorNode& baseNode, const ConstructorNode& targetNode)
{
    bool isSameNode = baseNode.childrenCount == targetNode.childrenCount
        && baseNode.shallowSize == targetNode.shallowSize
        && baseNode.retainedSize == targetNode.retainedSize;
    if (isSameNode) {
        auto baseChildren = ExpandConstructorNode(baseId, baseNode.id, 0, baseNode.childrenCount).children;
        auto targetChildren = ExpandConstructorNode(targetId, targetNode.id, 0, targetNode.childrenCount).children;
        std::unordered_set<uint64_t> childIds;
        for (const auto& child : baseChildren) {
            childIds.emplace(child.id);
        }
        for (const auto& child : targetChildren) {
            if (childIds.find(child.id) == childIds.end()) {
                isSameNode = false;
                break;
            }
        }
    }
    return isSameNode;
}

std::vector<ConstructorDiffNode> QuerySnapshotComparison(uint64_t baseId, uint64_t targetId)
{
    if (auto it = SnapshotComparison.find({baseId, targetId}); it != SnapshotComparison.end()) {
        return it->second;
    }
    std::unordered_set<std::string> allClassNames;
    std::unordered_map<std::string, ConstructorNode> baseConsNodes;
    uint32_t baseTotalSize = 0;
    uint32_t targetTotalSize = 0;
    for (auto& conNode : GetConstructorNodesBySnapshotID(baseId)) {
        baseConsNodes.emplace(conNode.className, conNode);
        allClassNames.emplace(conNode.className);
        baseTotalSize = conNode.totalSize;
    }
    std::unordered_map<std::string, ConstructorNode> targetConsNodes;
    for (auto& conNode : GetConstructorNodesBySnapshotID(targetId)) {
        targetConsNodes.emplace(conNode.className, conNode);
        allClassNames.emplace(conNode.className);
        targetTotalSize = conNode.totalSize;
    }
    std::vector<ConstructorDiffNode> res;
    for (auto& className : allClassNames) {
        auto baseRes = baseConsNodes.find(className);
        auto targetRes = targetConsNodes.find(className);
        if (baseRes != baseConsNodes.end() && targetRes != targetConsNodes.end()) {
            const auto& baseNode = baseRes->second;
            const auto& targetNode = targetRes->second;
            bool isSameNode = isSameConstructorNode(baseId, targetId, baseNode, targetNode);
            if (!isSameNode) {
                ConstructorDiffNode diffNode(baseNode, baseNode.childrenCount, targetNode.childrenCount,
                    baseNode.shallowSize, targetNode.shallowSize, baseTotalSize, targetTotalSize);
                res.emplace_back(diffNode);
            }
        } else if (baseRes != baseConsNodes.end()) {
            auto baseNode = baseRes->second;
            ConstructorDiffNode diffNode(baseNode, baseNode.childrenCount, 0,
                baseNode.shallowSize, 0, baseTotalSize, targetTotalSize);
            res.emplace_back(diffNode);
        } else {
            auto targetNode = targetRes->second;
            ConstructorDiffNode diffNode(targetNode, 0, targetNode.childrenCount,
                0, targetNode.shallowSize, baseTotalSize, targetTotalSize);
            res.emplace_back(diffNode);
        }
    }
    updateComparisonCache(baseId, targetId, res);
    return res;
}

template <typename T>
std::vector<T> getSubRange(const std::vector<T>& vec, size_t index, size_t length)
{
    size_t start = std::min(index, vec.size());
    size_t validLen = std::min(length, vec.size() - start);
    return {vec.begin() + start, vec.begin() + start + validLen};
}

static std::tuple<std::vector<InstanceNode>, std::vector<bool>, uint32_t> buildDiffChildrenAndStates(
    const std::vector<InstanceNode>& baseChildren,
    const std::vector<InstanceNode>& targetChildren,
    uint32_t startIndex, uint32_t length)
{
    std::unordered_set<uint64_t> baseChildIds;
    std::unordered_set<uint64_t> targetChildIds;
    for (auto& child : baseChildren) baseChildIds.emplace(child.id);
    for (auto& child : targetChildren) targetChildIds.emplace(child.id);

    std::vector<InstanceNode> diffNodes;
    std::vector<bool> states;
    for (const auto& node : baseChildren) {
        if (!targetChildIds.count(node.id)) {
            diffNodes.emplace_back(node);
            states.push_back(true);
        }
    }
    for (const auto& node : targetChildren) {
        if (!baseChildIds.count(node.id)) {
            diffNodes.emplace_back(node);
            states.push_back(false);
        }
    }

    return {
        getSubRange(diffNodes, startIndex, length),
        getSubRange(states, startIndex, length),
        static_cast<uint32_t>(diffNodes.size())
    };
}

ConstructorDiffNode ExpandConstructorDiffNode(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length)
{
    ConstructorNode baseNode = ConstructorNode();
    ConstructorNode targetNode = ConstructorNode();
    uint32_t baseTotalSize = 0;
    uint32_t targetTotalSize = 0;
    for (auto& conNode : GetConstructorNodesBySnapshotID(baseSnapshotId)) {
        baseTotalSize = conNode.totalSize;
        if (conNode.id == nodeId) {
            baseNode = conNode;
            break;
        }
    }
    for (auto& conNode : GetConstructorNodesBySnapshotID(targetSnapshotId)) {
        targetTotalSize = conNode.totalSize;
        if (conNode.id == nodeId) {
            targetNode = conNode;
            break;
        }
    }

    bool isBaseValid = (baseNode.id == nodeId);
    bool isTargetValid = (targetNode.id == nodeId);

    uint32_t baseChildrenCount = isBaseValid ? baseNode.childrenCount : 0;
    uint32_t targetChildrenCount = isTargetValid ? targetNode.childrenCount : 0;
    uint32_t baseShallowSize = isBaseValid ? baseNode.shallowSize : 0;
    uint32_t targetShallowSize = isTargetValid ? targetNode.shallowSize : 0;

    std::vector<InstanceNode> children;
    std::vector<bool> childAddedStates;
    uint32_t childrenCount = 0;
    if (isBaseValid && isTargetValid) {
        auto baseChildren = ExpandConstructorNode(
            baseSnapshotId, baseNode.id, 0, baseNode.childrenCount).children;
        auto targetChildren = ExpandConstructorNode(
            targetSnapshotId, targetNode.id, 0, targetNode.childrenCount).children;
        std::tie(children, childAddedStates, childrenCount) = buildDiffChildrenAndStates(
            baseChildren, targetChildren, startIndex, length);
    } else if (isTargetValid) {
        childrenCount = targetNode.childrenCount;
        auto targetChildren = ExpandConstructorNode(
            targetSnapshotId, targetNode.id, 0, targetNode.childrenCount).children;
        children = getSubRange(targetChildren, startIndex, length);
        std::vector<bool> states(targetChildren.size(), false);
        childAddedStates = getSubRange(states, startIndex, length);
    } else {
        childrenCount = baseNode.childrenCount;
        auto baseChildren = ExpandConstructorNode(
            baseSnapshotId, baseNode.id, 0, baseNode.childrenCount).children;
        children = getSubRange(baseChildren, startIndex, length);
        std::vector<bool> states(baseChildren.size(), true);
        childAddedStates = getSubRange(states, startIndex, length);
    }

    ConstructorDiffNode diffNode(
        isBaseValid ? baseNode : targetNode,
        baseChildrenCount,
        targetChildrenCount,
        baseShallowSize,
        targetShallowSize,
        baseTotalSize,
        targetTotalSize
    );
    diffNode.children = children;
    diffNode.childAddedStates = childAddedStates;
    diffNode.childrenCount = childrenCount;
    diffNode.startPosition = startIndex;
    diffNode.endPosition = startIndex + length - 1;
    return diffNode;
}

enum class ExpandDiffType : uint64_t {
    ExpandInstanceDiffNode = 1,
    ExpandDetailDiffNode = 2
};

InstanceDiffNode ExpandDiffNode(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId,
    uint32_t startIndex, uint32_t length, bool isReference, ExpandDiffType expendDiffType)
{
    bool added = true;
    auto baseSnapshotRes = RawHeapSnapshotDatas.find(baseSnapshotId);
    auto targetSnapshotRes = RawHeapSnapshotDatas.find(targetSnapshotId);
    if (baseSnapshotRes == RawHeapSnapshotDatas.end() || targetSnapshotRes == RawHeapSnapshotDatas.end()) {
        return InstanceDiffNode();
    }
    const auto& baseSnapshot = baseSnapshotRes->second;
    const auto& targetSnapshot = targetSnapshotRes->second;
    if (targetSnapshot.insNodeIdToIndex.count(nodeId)) {
        added = false;
    }
    uint32_t addedCount = 0;
    uint32_t removedCount = 0;
    uint32_t addedSize = 0;
    uint32_t removedSize = 0;
    InstanceNode insNode = InstanceNode();
    if (expendDiffType == ExpandDiffType::ExpandInstanceDiffNode) {
        insNode = ExpandInstanceNode(added? baseSnapshotId: targetSnapshotId, nodeId, startIndex, length);
    } else if (expendDiffType == ExpandDiffType::ExpandDetailDiffNode) {
        insNode = ExpandDetailNode(added? baseSnapshotId: targetSnapshotId, nodeId, isReference, startIndex, length);
    }
    bool found = false;
    if (auto item = SnapshotDiffs.find({baseSnapshotId, targetSnapshotId}); item != SnapshotDiffs.end()) {
        auto allConstructorDiffData = item->second;
        if (auto entry = allConstructorDiffData.find(insNode.className); entry != allConstructorDiffData.end()) {
            found = true;
            auto constructorDiffData = entry->second;
            addedCount = constructorDiffData.addedCount;
            removedCount = constructorDiffData.removedCount;
            addedSize = constructorDiffData.addedSize;
            removedSize = constructorDiffData.removedSize;
        }
    } else {
        resetSnapshotDiffs(baseSnapshotId, targetSnapshotId);
    }
    if (!found) {
        for (auto &diffNode : QuerySnapshotComparison(baseSnapshotId, targetSnapshotId)) {
            if (diffNode.className == insNode.className) {
                found = true;
                addedCount = diffNode.addedCount;
                removedCount = diffNode.removedCount;
                addedSize = diffNode.addedSize;
                removedSize = diffNode.removedSize;
                break;
            }
        }
        if (found) {
            SnapshotDiffs[{baseSnapshotId, targetSnapshotId}][insNode.className] =
                {addedCount, removedCount, addedSize, removedSize};
            SnapshotDiffs[{targetSnapshotId, baseSnapshotId}][insNode.className] =
                {removedCount, addedCount, removedSize, addedSize};
        }
    }
    InstanceDiffNode diffNode(insNode, addedCount, removedCount, addedSize, removedSize, added);
    return diffNode;
}

InstanceNode ExpandInstanceNode(
    uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length, bool needFields, bool needReference)
{
    if (auto item = RawHeapSnapshotDatas.find(snapshotId); item != RawHeapSnapshotDatas.end()) {
        auto& snapshot = item->second;
        auto idIt = snapshot.insNodeIdToIndex.find(nodeId);
        if (idIt == snapshot.insNodeIdToIndex.end()) {
            return InstanceNode();
        }
        auto insNode = snapshot.insNodes[snapshot.insNodeIdToIndex[nodeId]];
        auto res = snapshot.CreateInstanceNode(insNode.nodeIndex);
        res.startPosition = startIndex;
        res.endPosition = startIndex + length - 1;
        if (needFields) {
            for (auto id : getSubRange(insNode.children, startIndex, length)) {
                if (id >= snapshot.insNodes.size()) {
                    continue;
                }
                res.children.emplace_back(ExpandDetailNode(snapshotId, snapshot.insNodes[id].id, false, 0, 0));
            }
        }
        if (needReference) {
            for (auto id : getSubRange(insNode.references, startIndex, length)) {
                if (id >= snapshot.insNodes.size()) {
                    continue;
                }
                res.retainerNodes.emplace_back(ExpandDetailNode(snapshotId, snapshot.insNodes[id].id, false, 0, 0));
            }
        }
        return res;
    }
    return InstanceNode();
}

InstanceNode ExpandInstanceNode(
    uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length)
{
    return ExpandInstanceNode(snapshotId, nodeId, startIndex, length, true, true);
}

InstanceDiffNode ExpandInstanceDiffNode(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length)
{
    return ExpandDiffNode(baseSnapshotId, targetSnapshotId, nodeId,
        startIndex, length, false, ExpandDiffType::ExpandInstanceDiffNode);
}

InstanceNode ExpandDetailNode(
    uint64_t snapshotId, uint64_t nodeId, bool isReference, uint32_t startIndex, uint32_t length)
{
    return ExpandInstanceNode(snapshotId, nodeId, startIndex, length, !isReference, isReference);
}

InstanceDiffNode ExpandDetailDiffNode(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId,
    bool isReference, uint32_t startIndex, uint32_t length)
{
    return ExpandDiffNode(baseSnapshotId, targetSnapshotId, nodeId,
        startIndex, length, isReference, ExpandDiffType::ExpandDetailDiffNode);
}

std::vector<ConstructorNode> GetRootNodesBySnapshotID(uint64_t id, std::set<uint8_t> rootTypes)
{
    if (rootTypes.empty()) {
        return {};
    }

    auto it = RawHeapSnapshotDatas.find(id);
    if (it == RawHeapSnapshotDatas.end()) {
        return {};
    }

    return it->second.GetRootNodes(rootTypes);
}

std::vector<ConstructorDiffNode> GetRootDiffNodesBySnapshotID(
    uint64_t baseSnapshotId, uint64_t targetSnapshotId, std::set<uint8_t> rootTypes)
{
    std::unordered_map<uint64_t, ConstructorDiffNode> allDiffNodes;
    for (auto& conNode : QuerySnapshotComparison(baseSnapshotId, targetSnapshotId)) {
        allDiffNodes.emplace(conNode.id, conNode);
    }
    auto baseRoots = GetRootNodesBySnapshotID(baseSnapshotId, rootTypes);
    auto targetRoots = GetRootNodesBySnapshotID(targetSnapshotId, rootTypes);
    std::vector<ConstructorDiffNode> res;
    std::unordered_map<uint64_t, ConstructorNode> baseRootMap;
    std::unordered_map<uint64_t, ConstructorNode> targetRootMap;
    for (auto& node : baseRoots) baseRootMap.emplace(node.id, node);
    for (auto& node : targetRoots) targetRootMap.emplace(node.id, node);
    for (auto& conNode : baseRoots) {
        if (allDiffNodes.count(conNode.id)) {
            auto node = allDiffNodes.find(conNode.id)->second;
            node.childrenCount += conNode.childrenCount;
            node.children = conNode.children;
            if (targetRootMap.count(conNode.id)) {
                auto targetNode = targetRootMap.find(conNode.id)->second;
                node.children.insert(node.children.end(),
                    make_move_iterator(targetNode.children.begin()),
                    make_move_iterator(targetNode.children.end()));
                node.childrenCount += targetNode.childrenCount;
            }
            res.emplace_back(node);
        }
    }
    for (auto& conNode : targetRoots) {
        if (allDiffNodes.count(conNode.id) && !baseRootMap.count(conNode.id)) {
            auto node = allDiffNodes.find(conNode.id)->second;
            node.childrenCount += conNode.childrenCount;
            node.children = conNode.children;
            res.emplace_back(node);
        }
    }
    return res;
}

std::vector<ThreadInfo> GetThreadInfos(uint64_t snapshotId)
{
    auto it = RawHeapSnapshotDatas.find(snapshotId);
    if (it == RawHeapSnapshotDatas.end()) {
        return {};
    }

    return it->second.GetThreadInfos();
}

} // namespace Cjprof