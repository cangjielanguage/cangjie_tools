// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef HEAP_ANALYZER_H
#define HEAP_ANALYZER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include "Utility/Singleton.h"
#include "Data/Hprof.h"

/**
 * Information of RawHeapSnapshot
 * nodeCount: number of nodes
 * edgeCount: number of edges
 * nodes: the set of nodes
 * edges: the set of edges
 * strings: full string content in heap memory snapshots, STRING
 * startTime: sampling time
 * threads: the set of threads
 */
class RawHeapSnapshot {
public:
    enum NodeType : uint16_t {
        HIDDEN = 0,
        ARRAY = 1,
        STRING = 2,
        OBJECT = 3,
        CODE,
        CLOSURE,
        REGEXP,
        NUMBER,
        NATIVE,
        SYNTHETIC,
        CONCATENATED_STRING,
        SLICED_STRING,
        SYMBOL,
        BIGINT,
        UNKNOWN_TYPE
    };
    enum RootType : uint16_t {
        HYPHEN = 0, // -
        GLOBAL = 1,
        LOCAL = 2,
        UNKNOWN = 3
    };
    /**
     * Information of the heap node
     * type: heap node type
     * nameIndex: heap node type name
     * id: heap node id
     * selfSize: shallow size of heap node
     * edgeCount: number of out edges
     */
    struct Node {
        uint16_t type;
        uint16_t rootType;
        uint32_t nameIndex;
        uint64_t id;
        uint32_t selfSize;
        uint32_t edgeCount;
        uint32_t nodeIndex;
        uint32_t arrayLen;
        bool IsRootGlobal() const {
            return rootType == RootType::GLOBAL;
        }
        bool IsRootLocal() const {
            return rootType == RootType::LOCAL;
        }
        bool IsRootUnknown() const {
            return rootType == RootType::UNKNOWN;
        }
        bool IsRoot() const {
            return rootType != RootType::HYPHEN;
        }
        std::string TypeString() const {
            const char* nodeTypeNames[NodeType::UNKNOWN_TYPE] = {
                "hidden", "array", "string", "object", "code",
                "closure", "regexp", "number", "native", "synthetic",
                "concatenated string", "sliced string", "symbol", "bigint"
            };
            if (type >= NodeType::UNKNOWN_TYPE) return "[unknown]";
            return "[" + std::string(nodeTypeNames[type]) + "]";
        }
        std::string RootString() const {
            if (IsRootGlobal()) {
                return "[ROOT GLOBAL]";
            }
            if (IsRootLocal()) {
                return "[ROOT LOCAL]";
            }
            if (IsRootUnknown()) {
                return "[ROOT UNKNOWN]";
            }
            return "[-]";
        }
    };
    /**
     * Information of the Edge
     * type: type of the node at the start point of the edge
     * nameOrIndex: name of the node referenced by this edge
     * toNode: index of the node referenced by this edge in the nodes
     */
    struct Edge {
        uint32_t type;
        uint32_t nameOrIndex;
        uint32_t toNode;
    };
    /**
     * Information of the stack frame
     * funcName: Function name corresponding to the stack frame
     * fileName: Source file name corresponding to the stack frame
     * line: Source code line number corresponding to the stack frame
     * locals: The set of local node objects corresponding to the stack frame
     * id: Frame id
     */
    struct Frame {
        uint32_t funcName;
        uint32_t fileName;
        int line;
        std::vector<uint32_t> locals;
    };
    /**
     * Information of the thread
     * name: Cangjie thread name
     * frames: The set of stack frames for this thread
     * id: Thread id
     */
    struct Thread {
        std::string name;
        std::vector<Frame> frames;
    };
    uint32_t nodeCount;
    uint32_t edgeCount;
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::vector<std::string> strings;
    uint64_t startTime;
    uint64_t hashid;
    std::string filePath;
    std::vector<Thread> threads;
    uint32_t fileSize;
    void PrintSummary();
};

class HeapAnalyzer : public Singleton<HeapAnalyzer> {
public:
    HeapAnalyzer() = default;
    bool SetData(const std::string &file);
    bool Analyze(bool verbose);
    void ShowThread();
    void ShowObject();
    void ShowReference(const std::string &objNameList, int maxDepth, bool incoming = false);
    RawHeapSnapshot GetRawHeapSnapshot();

private:
    struct Object {
        Hprof::ID id;
        std::string name;
        uint64_t size = 0;
        uint64_t retainedSize = 0;
        std::unordered_set<Hprof::ID> outRef;
        std::unordered_set<Hprof::ID> inRef;

        Object(Hprof::ID id)
        {
            this->id = id;
        }
    };

    struct Frame {
        std::string name;
        std::string fileName;
        int line;
        std::vector<std::shared_ptr<Object>> locals;
    };

    struct Thread {
        std::string name;
        std::vector<Frame> frames;
    };

    void AnalyzeObject();
    void AnalyzeInstance();
    void AnalyzeArray();
    void AnalyzeThread();
    std::shared_ptr<Object> GetObject(Hprof::ID id);

    inline uint64_t Align(uint64_t size)
    {
        /* Align size to 8 bytes. */
        return (size + 7) & ~(0x7ULL);
    }

    std::string m_data;
    Hprof m_hprof;
    std::vector<std::shared_ptr<Object>> m_objects;
    std::vector<Thread> m_threads;
    uint32_t m_fileSize;
    std::unordered_map<Hprof::ID, std::shared_ptr<HeapAnalyzer::Object>> objects_cache;
};

#endif // HEAP_ANALYZER_H