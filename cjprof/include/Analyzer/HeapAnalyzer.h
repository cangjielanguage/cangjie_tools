// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef HEAP_ANALYZER_H
#define HEAP_ANALYZER_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include "Utility/Singleton.h"
#include "Data/Hprof.h"

class HeapAnalyzer : public Singleton<HeapAnalyzer> {
public:
    HeapAnalyzer() = default;
    bool SetData(const std::string &file);
    bool Analyze(bool verbose);
    void ShowThread();
    void ShowObject();
    void ShowReference(const std::string &objNameList, int maxDepth, bool incoming = false);

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
    std::unordered_map<Hprof::ID, std::shared_ptr<HeapAnalyzer::Object>> objects_cache;
};

#endif // HEAP_ANALYZER_H