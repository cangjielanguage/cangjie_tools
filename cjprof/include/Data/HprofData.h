// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef HPROF_DATA_H
#define HPROF_DATA_H

// Pure data structure - no parsing logic
// Separates data model from parser for better organization

#include "ObjectCategory.h"
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

struct HprofData {
    using u1 = uint8_t;
    using u2 = uint16_t;
    using u4 = uint32_t;
    using u8 = uint64_t;
    using ID = uint64_t;

    enum BasicType : u1 {
        OBJECT = 2,
        BOOLEAN = 4,
        SHORT = 9,
        INT = 10,
        LONG = 11
    };

    struct Class {
        ID name;
        u4 size;
    };

    struct Instance {
        ID cls;
        std::vector<ID> fields;
    };

    struct Array {
        BasicType type;
        ID cls;
        u4 num;
        std::vector<ID> elements;
        u4 GetFixedSize() const { return 16; }
    };

    struct Frame {
        ID name;
        ID fileName;
        u4 line;

        bool operator<(const Frame &rhs) const
        {
            if (name != rhs.name) return name < rhs.name;
            if (fileName != rhs.fileName) return fileName < rhs.fileName;
            return line < rhs.line;
        }
    };

    struct StackTrace {
        u4 thread;
        std::vector<ID> frames;

        bool operator<(const StackTrace &rhs) const
        {
            if (thread != rhs.thread) return thread < rhs.thread;
            if (frames.size() != rhs.frames.size()) return frames.size() < rhs.frames.size();
            for (size_t i = 0; i < frames.size(); i++) {
                if (frames[i] != rhs.frames[i]) return frames[i] < rhs.frames[i];
            }
            return false;
        }
    };

    struct Thread {
        ID name;
        u4 idx;
        u4 stackTraceIdx;
    };

    struct Local {
        u4 thread;
        u4 frame;
    };

    struct Sample {
        std::map<u4, u4> stackTraces;
    };

    // Data containers
    std::map<ID, std::string> strings;
    std::map<ID, Frame> frames;
    std::map<u4, StackTrace> stackTraces;
    std::map<ID, Thread> threads;
    std::unordered_map<ID, Class> classes;
    std::map<ID, Instance> instances;
    std::unordered_map<ID, Array> arrays;
    std::unordered_map<ID, Local> locals;
    std::unordered_set<ID> globals;
    std::unordered_set<ID> unknown;
    std::map<u4, Sample> cpuSamples;
    std::unordered_map<ID, u4> componentNums;
    std::unordered_map<ID, ObjectCategory> objectCategories;

    u8 fileTime = 0;
    u4 idSize = 0;

    // Static empty containers for null state
    static const std::map<ID, std::string> emptyStrings;
    static const std::map<ID, Frame> emptyFrames;
    static const std::map<u4, StackTrace> emptyStackTraces;
    static const std::map<ID, Thread> emptyThreads;
    static const std::unordered_map<ID, Class> emptyClasses;
    static const std::map<ID, Instance> emptyInstances;
    static const std::unordered_map<ID, Array> emptyArrays;
    static const std::unordered_map<ID, Local> emptyLocals;
    static const std::unordered_set<ID> emptyGlobals;
    static const std::unordered_set<ID> emptyUnknown;
    static const std::map<u4, Sample> emptyCpuSamples;
    static const std::unordered_map<ID, u4> emptyComponentNums;
    static const std::unordered_map<ID, ObjectCategory> emptyObjectCategories;
};

#endif // HPROF_DATA_H