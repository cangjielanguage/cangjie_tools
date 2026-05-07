// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef HPROF_H
#define HPROF_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

class Hprof {
public:
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

        /* Fixed members ('_typeinfo', 'size') size of a raw array. */
        u4 GetFixedSize()
        {
            /* 16 bytes */
            return 16;
        }
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
        };
    };

    struct StackTrace {
        u4 thread;
        std::vector<ID> frames;

        bool operator<(const StackTrace &rhs) const
        {
            if (thread != rhs.thread) {
                return thread < rhs.thread;
            }

            if (frames.size() != rhs.frames.size()) {
                return frames.size() < rhs.frames.size();
            }

            for (size_t i = 0; i < frames.size() && i < rhs.frames.size(); i++) {
                if (frames[i] != rhs.frames[i]) {
                    return frames[i] < rhs.frames[i];
                }
            }

            return false;
        };
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

    Hprof() = default;
    bool Parse(const std::string &data, bool verbose = false);
    const std::string &GenData();
    void PutCpuSample(const std::vector<std::string> &callChain, u4 period);

    const std::map<ID, std::string> &GetStrings() const
    {
        return m_strings;
    }

    const std::map<ID, Frame> &GetFrames() const
    {
        return m_frames;
    }

    const std::map<u4, StackTrace> &GetStackTraces() const
    {
        return m_stackTraces;
    }

    const std::map<ID, Thread> &GetThreads() const
    {
        return m_threads;
    }

    const std::map<ID, Class> &GetClasses() const
    {
        return m_classes;
    }

    const std::map<ID, Instance> &GetInstances() const
    {
        return m_instances;
    }

    const std::unordered_map<ID, Array> &GetArrays() const
    {
        return m_arrays;
    }

    const std::unordered_map<ID, Local> &GetLocalsRoots() const
    {
        return m_locals;
    }

    const std::unordered_set<ID> &GetGlobalsRoots() const
    {
        return m_globals;
    }

    const std::unordered_set<ID> &GetUnknownRoots() const
    {
        return m_unknown;
    }

    const std::map<u4, Sample> &GetCpuSamples() const
    {
        return m_cpuSamples;
    }

    const std::unordered_map<ID, u4> &GetComponentNums() const
    {
        return m_componentNums;
    }

    const u8 GetFileTime() const
    {
        return m_fileTime;
    }

private:
    enum Tag : u1 {
        STRING = 0x01,
        CLASS = 0x02,
        STACK_FRAME = 0x04,
        STACK_TRACE = 0x05,
        START_THREAD = 0x0a,
        HEAP_DUMP = 0x0c,
        CPU_SAMPLES = 0x0d
    };

    enum HeapDumpSubTag : u1 {
        ROOT_GLOBAL = 0x01,
        ROOT_LOCAL = 0x02,
        CLASS_DUMP = 0x20,
        INSTANCE_DUMP = 0x21,
        OBJECT_ARRAY_DUMP = 0x22,
        PRIMITIVE_ARRAY_DUMP = 0x23,
        STRUCT_ARRAY_DUMP = 0x24,
        PINNED_INSTANCE_DUMP = 0x25,
        LARGE_INSTANCE_DUMP = 0x26,
        LARGE_OBJECT_ARRAY_DUMP = 0x27,
        LARGE_PRIMITIVE_ARRAY_DUMP = 0x28,
        LARGE_STRUCT_ARRAY_DUMP = 0x29,
        UNMOVABLE_INSTANCE_DUMP = 0x2A,
        UNMOVABLE_OBJECT_ARRAY_DUMP = 0x2B,
        UNMOVABLE_PRIMITIVE_ARRAY_DUMP = 0x2C,
        UNMOVABLE_STRUCT_ARRAY_DUMP = 0x2D,
        ROOT_UNKNOWN = 0xff
    };

#pragma pack(push, 1)
    struct FileHeader {
        char ident[22];
        u4 idSize;
        u4 timeHigh;
        u4 timeLow;
    };

    struct RecordHeader {
        Tag tag;
        u8 length;
    };

    struct StringRecord : RecordHeader {
        ID id;      // ID for this string
        char str[]; // UTF8 characters for string (NOT NULL terminated)
    };

    struct LoadClassRecord : RecordHeader {
        ID id;      // class object ID
        ID name;    // class name string ID
    };

    struct StackFrameRecord : RecordHeader {
        ID id;          // stack frame ID
        ID name;        // method name string ID
        ID fileName;    // source file name string ID
        u4 line;        // line number
    };

    struct StackTraceRecord : RecordHeader {
        u4 idx;         // stack trace serial number
        u4 thread;      // thread serial number
        u4 frameNum;    // number of frames
        ID frames[];    // series of stack frame ID's
    };

    struct StartThreadRecord : RecordHeader {
        u4 idx;             // thread serial number
        ID id;              // thread object ID
        u4 stackTraceIdx;   // stack trace serial number
        ID name;            // thread name string ID
    };

    struct CpuSamplesRecord : RecordHeader {
        u4 period;              // 采样周期
        u4 num;                 // 样本个数
        struct {
            u4 num;             // 采样次数
            u4 stackTraceIdx;   // 调用栈序号
        } samples[];
    };
#pragma pack(pop)

    void ParseString(bool verbose);
    void ParseClass(bool verbose);
    void ParseStackFrame(bool verbose);
    void ParseStackTrace(bool verbose);
    void ParseStartThread(bool verbose);
    void ParseHeapDump(bool verbose);
    void ParseHeapDumpRootGlobal(bool verbose);
    void ParseHeapDumpRootLocal(bool verbose);
    void ParseHeapDumpClassDump(bool verbose);
    void ParseHeapDumpInstanceDump(bool verbose);
    void ParseHeapDumpObjectArrayDump(bool verbose);
    void ParseHeapDumpPrimitiveArrayDump(bool verbose);
    void ParseHeapDumpStructArrayDump(bool verbose);
    void ParseHeapDumpRootUnknown(bool verbose);
    void ParseCpuSamples(bool verbose);
    void GenString(ID id, const std::string &str);
    void GenStackFrame(ID id, const Frame &frame);
    void GenStackTrace(u4 idx, const StackTrace &stackTrace);
    void GenCpuSample(u4 period, const Sample &sample);

    template <typename T>
    T *ReadAs()
    {
        return m_data.size() - m_curPos >= sizeof(T) ? (T *)(m_data.data() + m_curPos) : nullptr;
    }

    template <typename T>
    void WriteAs(const T &data)
    {
        m_data.append(std::string((const char *)&data, sizeof(T)));
    }

    inline u2 SwapEndian(u2 val)
    {
        return static_cast<short unsigned int>((val & 0xff00U) >> 8)
            | static_cast<short unsigned int>((val & 0xffU) << 8);
    }

    inline u4 SwapEndian(u4 val)
    {
        return ((val & 0xff000000U) >> 24) | ((val & 0xff0000U) >> 8) |
            ((val & 0xff00U) << 8) | ((val & 0xffU) << 24);
    }

    inline u8 SwapEndian(u8 val)
    {
        return ((val & 0xff00000000000000ULL) >> 56) | ((val & 0xff000000000000ULL) >> 40) |
            ((val & 0xff0000000000ULL) >> 24) | ((val & 0xff00000000ULL) >> 8) |
            ((val & 0xff000000ULL) << 8) | ((val & 0xff0000ULL) << 24) |
            ((val & 0xff00ULL) << 40) | ((val & 0xffULL) << 56);
    }

    template <typename T1, typename T2>
    std::shared_ptr<T1> GetValue(std::map<T2, std::shared_ptr<T1>> &container, T2 key)
    {
        auto &val = container[key];
        if (!val) {
            val = std::make_shared<T1>();
        }

        return val;
    }

    template <typename T1, typename T2>
    T2 GetMapValue(std::map<T1, T2> &map, const T1 &key)
    {
        static T2 val = 1;

        auto it = map.find(key);
        if (it != map.end()) {
            return it->second;
        }

        auto id = val++;
        map[key] = id;

        return id;
    }

private:
    static constexpr char m_headerFlag[] = "CANGJIE PROFILE 1.0.1";
    std::string m_data;
    size_t m_curPos;
    std::map<ID, std::string> m_strings;
    std::map<ID, Frame> m_frames;
    std::map<u4, StackTrace> m_stackTraces;
    std::map<ID, Thread> m_threads;
    std::map<ID, Class> m_classes;
    std::map<ID, Instance> m_instances;
    std::unordered_map<ID, Array> m_arrays;
    std::unordered_map<ID, Local> m_locals;
    std::unordered_set<ID> m_globals;
    std::unordered_set<ID> m_unknown;
    std::map<std::string, ID> m_stringsMap;
    std::map<Frame, ID> m_framesMap;
    std::map<StackTrace, u4> m_stackTracesMap;
    std::map<u4, Sample> m_cpuSamples;
    std::unordered_map<ID, u4> m_componentNums;
    u8 m_fileTime;
};

#endif // HPROF_H