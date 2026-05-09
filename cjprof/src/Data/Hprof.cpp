// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <fstream>
#include <iostream>
#include <cinttypes>
#include <functional>
#include <sys/time.h>
#include "Data/Hprof.h"

#pragma pack(push, 1)

bool Hprof::Parse(const std::string &data, bool verbose)
{
    m_data = data;
    m_curPos = 0;

    auto header = ReadAs<FileHeader>();
    if (!header || (std::string(header->ident) != m_headerFlag)) {
        fprintf(stderr, "error: Invalid file contents or file format.\n");
        return false;
    }

    m_fileTime = header->timeHigh;
    m_fileTime = (m_fileTime << 32) + header->timeLow;

    m_curPos += sizeof(FileHeader);

    std::map<Tag, std::function<void(bool)>> recordParsers = {
        { STRING, std::bind(&Hprof::ParseString, this, std::placeholders::_1) },
        { CLASS, std::bind(&Hprof::ParseClass, this, std::placeholders::_1) },
        { STACK_FRAME, std::bind(&Hprof::ParseStackFrame, this, std::placeholders::_1) },
        { STACK_TRACE, std::bind(&Hprof::ParseStackTrace, this, std::placeholders::_1) },
        { START_THREAD, std::bind(&Hprof::ParseStartThread, this, std::placeholders::_1) },
        { HEAP_DUMP, std::bind(&Hprof::ParseHeapDump, this, std::placeholders::_1) },
        { CPU_SAMPLES, std::bind(&Hprof::ParseCpuSamples, this, std::placeholders::_1) }
    };
    while (m_curPos < m_data.size()) {
        auto record = ReadAs<RecordHeader>();
        if (!record) {
            fprintf(stderr, "error: Record was truncated at offset 0x%zx\n", m_curPos);
            return false;
        }

        auto size = SwapEndian(record->length);
        if (m_curPos + size > m_data.size()) {
            fprintf(stderr, "error: Record length is too long at offset 0x%zx\n", m_curPos);
            return false;
        }

        auto it = recordParsers.find(record->tag);
        if (it != recordParsers.end()) {
            it->second(verbose);
            continue;
        }

        if (verbose) {
            printf("[SKIPED@0x%zx] tag = %u\n", m_curPos, record->tag);
        }

        m_curPos += sizeof(RecordHeader) + size;
    }

    if (m_curPos != m_data.size()) {
        fprintf(stderr, "error: Unknown record or the length of the last record is too long.\n");
        return false;
    }

    return true;
}

void Hprof::ParseString(bool verbose)
{
    auto rec = ReadAs<StringRecord>();
    if (rec == nullptr) {
        return;
    }
    ID id = SwapEndian(rec->id);
    auto length = SwapEndian(rec->length);
    if (length < sizeof(rec->id)) {
        fprintf(stderr, "error: StringRecord length too small at offset 0x%zx\n", m_curPos);
        return;
    }
    m_strings[id] = std::string(rec->str, length - sizeof(rec->id));
    if (verbose) {
        printf("[STRING@0x%zx] id = 0x%" PRIx64 ", str = \"%s\"\n", m_curPos, id, m_strings[id].c_str());
    }

    m_curPos += sizeof(RecordHeader) + SwapEndian(rec->length);
}

void Hprof::ParseClass(bool verbose)
{
    auto rec = ReadAs<LoadClassRecord>();
    if (rec == nullptr) {
        return;
    }
    ID id = SwapEndian(rec->id);
    m_classes[id].name = SwapEndian(rec->name);
    if (verbose) {
        printf("[LOAD CLASS@0x%zx] id = 0x%" PRIx64 ", name id = 0x%" PRIx64 "\n", m_curPos, id, m_classes[id].name);
    }

    m_curPos += sizeof(RecordHeader) + SwapEndian(rec->length);
}

void Hprof::ParseStackFrame(bool verbose)
{
    auto rec = ReadAs<StackFrameRecord>();
    if (rec == nullptr) {
        return;
    }
    ID id = SwapEndian(rec->id);
    m_frames[id] = { SwapEndian(rec->name), SwapEndian(rec->fileName), SwapEndian(rec->line) };
    if (verbose) {
        printf("[STACK FRAME@0x%zx] id = 0x%" PRIx64 ", method name id = 0x%" PRIx64 ", file name id = 0x%" PRIx64 ", line = %u\n",
            m_curPos, id, m_frames[id].name, m_frames[id].fileName, m_frames[id].line);
    }

    m_curPos += sizeof(RecordHeader) + SwapEndian(rec->length);
}

void Hprof::ParseStackTrace(bool verbose)
{
    auto rec = ReadAs<StackTraceRecord>();
    if (rec == nullptr) {
        return;
    }
    u4 idx = SwapEndian(rec->idx);
    m_stackTraces[idx].thread = SwapEndian(rec->thread);
    for (size_t i = 0; i < SwapEndian(rec->frameNum); i++) {
        m_stackTraces[idx].frames.push_back(SwapEndian(rec->frames[i]));
    }

    if (verbose) {
        printf("[STACK TRACE@0x%zx] idx = %u, thread idx = %u, frames = [", m_curPos, idx, m_stackTraces[idx].thread);

        auto frames = m_stackTraces[idx].frames;
        for (size_t i = 0; i < frames.size(); i++) {
            printf("0x%" PRIx64 "%s", frames[i], i < frames.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }

    m_curPos += sizeof(RecordHeader) + SwapEndian(rec->length);
}

void Hprof::ParseStartThread(bool verbose)
{
    auto rec = ReadAs<StartThreadRecord>();
    if (rec == nullptr) {
        return;
    }
    ID id = SwapEndian(rec->id);
    m_threads[id].idx = SwapEndian(rec->idx);
    m_threads[id].stackTraceIdx = SwapEndian(rec->stackTraceIdx);
    m_threads[id].name = SwapEndian(rec->name);
    if (verbose) {
        printf("[START THREAD@0x%zx] id = 0x%" PRIx64 ", thread idx = %u, stack trace idx = %u, name id = 0x%" PRIx64 "\n",
            m_curPos, id, m_threads[id].idx, m_threads[id].stackTraceIdx, m_threads[id].name);
    }

    m_curPos += sizeof(RecordHeader) + SwapEndian(rec->length);
}

void Hprof::ParseHeapDump(bool verbose)
{
    std::unordered_map<HeapDumpSubTag, std::function<void(bool)>> subRecordParsers = {
        { ROOT_GLOBAL, std::bind(&Hprof::ParseHeapDumpRootGlobal, this, std::placeholders::_1) },
        { ROOT_LOCAL, std::bind(&Hprof::ParseHeapDumpRootLocal, this, std::placeholders::_1) },
        { CLASS_DUMP, std::bind(&Hprof::ParseHeapDumpClassDump, this, std::placeholders::_1) },
        { INSTANCE_DUMP, std::bind(&Hprof::ParseHeapDumpInstanceDump, this, std::placeholders::_1) },
        { OBJECT_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpObjectArrayDump, this, std::placeholders::_1) },
        { PRIMITIVE_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpPrimitiveArrayDump, this, std::placeholders::_1) },
        { STRUCT_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpStructArrayDump, this, std::placeholders::_1) },
        { PINNED_INSTANCE_DUMP, std::bind(&Hprof::ParseHeapDumpInstanceDump, this, std::placeholders::_1) },
        { LARGE_INSTANCE_DUMP, std::bind(&Hprof::ParseHeapDumpInstanceDump, this, std::placeholders::_1) },
        { LARGE_OBJECT_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpObjectArrayDump, this, std::placeholders::_1) },
        { LARGE_PRIMITIVE_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpPrimitiveArrayDump, this, std::placeholders::_1) },
        { LARGE_STRUCT_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpStructArrayDump, this, std::placeholders::_1) },
        { UNMOVABLE_INSTANCE_DUMP, std::bind(&Hprof::ParseHeapDumpInstanceDump, this, std::placeholders::_1) },
        { UNMOVABLE_OBJECT_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpObjectArrayDump, this, std::placeholders::_1) },
        { UNMOVABLE_PRIMITIVE_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpPrimitiveArrayDump, this, std::placeholders::_1) },
        { UNMOVABLE_STRUCT_ARRAY_DUMP, std::bind(&Hprof::ParseHeapDumpStructArrayDump, this, std::placeholders::_1) },
        { ROOT_UNKNOWN, std::bind(&Hprof::ParseHeapDumpRootUnknown, this, std::placeholders::_1) }
    };
    auto header = ReadAs<RecordHeader>();
    if (header == nullptr) {
        return;
    }
    m_curPos += sizeof(RecordHeader);
    auto endPos = m_curPos + SwapEndian(header->length);
    while (m_curPos < endPos) {
        auto tag = ReadAs<HeapDumpSubTag>();
        if (tag == nullptr) {
            return;
        }
        auto it = subRecordParsers.find(*tag);
        if (it != subRecordParsers.end()) {
            it->second(verbose);
        } else {
            fprintf(stderr, "error: Unknown sub-tag %u at offset 0x%zx\n", *tag, m_curPos);
            return;
        }
    }
}

void Hprof::ParseHeapDumpRootGlobal(bool verbose)
{
    struct RootGlobal {
        u1 tag;
        ID objId;   // object ID
    } *rec = ReadAs<RootGlobal>();
    if (!rec) {
        m_curPos += sizeof(RootGlobal);
        return;
    }

    ID id = SwapEndian(rec->objId);
    m_globals.insert(id);

    if (verbose) {
        printf("[ROOT GLOBAL@0x%zx] id = 0x%" PRIx64 "\n", m_curPos, id);
    }

    m_curPos += sizeof(RootGlobal);
}

void Hprof::ParseHeapDumpRootLocal(bool verbose)
{
    struct RootLocal {
        u1 tag;
        ID id;      // object ID
        u4 thread;  // thread serial number
        u4 frame;   // frame number in stack trace (-1 for empty)
    } *rec = ReadAs<RootLocal>();
    if (!rec) {
        m_curPos += sizeof(RootLocal);
        return;
    }

    ID id = SwapEndian(rec->id);
    m_locals[id] = { SwapEndian(rec->thread), SwapEndian(rec->frame) };
    if (verbose) {
        printf("[ROOT LOCAL@0x%zx] id = 0x%" PRIx64 ", thread idx = %u, frame = %u\n",
            m_curPos, id, m_locals[id].thread, m_locals[id].frame);
    }

    m_curPos += sizeof(RootLocal);
}

void Hprof::ParseHeapDumpClassDump(bool verbose)
{
    struct ClassDump {
        u1 tag;
        ID id;              // class object ID
        u4 size;            // instance size (in bytes)
    } *rec = ReadAs<ClassDump>();
    if (!rec) {
        m_curPos += sizeof(ClassDump);
        return;
    }

    auto size = sizeof(ClassDump);
    if (m_curPos + size > m_data.size()) {
        m_curPos += size;
        return;
    }

    ID id = SwapEndian(rec->id);
    m_classes[id].size = SwapEndian(rec->size);
    if (verbose) {
        printf("[CLASS DUMP@0x%zx] id = 0x%" PRIx64 ", inst size = %u\n", m_curPos, id, m_classes[id].size);
    }

    m_curPos += sizeof(ClassDump);
}

void Hprof::ParseHeapDumpInstanceDump(bool verbose)
{
    struct InstanceDump {
        u1 tag;
        ID id;          // object ID
        ID cls;         // class object ID
        u4 num;         // number of bytes that follow
        u8 values[];    // instance field values
    } *rec = ReadAs<InstanceDump>();
    if (!rec) {
        m_curPos += sizeof(InstanceDump);
        return;
    }

    auto num = SwapEndian(rec->num);
    auto size = sizeof(InstanceDump) + sizeof(u8) * num;
    if (m_curPos + size > m_data.size()) {
        m_curPos += size;
        return;
    }

    ID id = SwapEndian(rec->id);
    m_instances[id].cls = SwapEndian(rec->cls);
    for (size_t i = 0; i < num; i++) {
        m_instances[id].fields.push_back(SwapEndian(rec->values[i]));
    }

    if (verbose) {
        printf("[INSTANCE DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%" PRIx64 ", value = [", m_curPos, id, m_instances[id].cls);

        auto fields = m_instances[id].fields;
        for (size_t i = 0; i < fields.size(); i++) {
            printf("0x%" PRIx64 "%s", fields[i], i < fields.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }

    m_curPos += size;
}

void Hprof::ParseHeapDumpObjectArrayDump(bool verbose)
{
    struct ObjectArrayDump {
        u1 tag;
        ID id;          // array object ID
        u4 num;         // number of elements
        ID cls;         // array class object ID
        ID elements[];  // elements
    } *rec = ReadAs<ObjectArrayDump>();
    if (!rec) {
        m_curPos += sizeof(ObjectArrayDump);
        return;
    }

    auto num = SwapEndian(rec->num);
    auto size = sizeof(ObjectArrayDump) + sizeof(ID) * num;
    if (m_curPos + size > m_data.size()) {
        m_curPos += size;
        return;
    }

    ID id = SwapEndian(rec->id);
    m_arrays[id].type = OBJECT;
    m_arrays[id].cls = SwapEndian(rec->cls);
    m_arrays[id].num = num;
    for (size_t i = 0; i < num; i++) {
        m_arrays[id].elements.push_back(SwapEndian(rec->elements[i]));
    }

    if (verbose) {
        printf("[OBJECT ARRAY DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%" PRIx64 ", elements = [", m_curPos, id, m_arrays[id].cls);

        auto elements = m_arrays[id].elements;
        for (size_t i = 0; i < m_arrays[id].num; i++) {
            printf("0x%" PRIx64 "%s", elements[i], i < elements.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }

    m_curPos += size;
}

void Hprof::ParseHeapDumpStructArrayDump(bool verbose)
{
    struct StructArrayDump {
        u1 tag;
        ID id;          // array object ID
        u4 componentNum; // number of components, which is less than or equal to num
        u4 num;         // number of ref fields
        ID cls;         // array class object ID
        ID elements[];  // elements
    } *rec = ReadAs<StructArrayDump>();
    if (!rec) {
        m_curPos += sizeof(StructArrayDump);
        return;
    }

    auto num = SwapEndian(rec->num);
    auto size = sizeof(StructArrayDump) + sizeof(ID) * num;
    if (m_curPos + size > m_data.size()) {
        m_curPos += size;
        return;
    }

    ID id = SwapEndian(rec->id);
    m_arrays[id].type = OBJECT;
    m_arrays[id].cls = SwapEndian(rec->cls);
    m_arrays[id].num = num;
    m_componentNums[id] = SwapEndian(rec->componentNum);
    for (size_t i = 0; i < num; i++) {
        m_arrays[id].elements.push_back(SwapEndian(rec->elements[i]));
    }

    if (verbose) {
        printf("[STRUCT ARRAY DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%" PRIx64 ", elements = [", m_curPos, id, m_arrays[id].cls);

        auto elements = m_arrays[id].elements;
        for (size_t i = 0; i < m_arrays[id].num; i++) {
            printf("0x%" PRIx64 "%s", elements[i], i < elements.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }

    m_curPos += size;
}

void Hprof::ParseHeapDumpPrimitiveArrayDump(bool verbose)
{
    struct PrimitiveArrayDump {
        u1 tag;
        ID id;          // array object ID
        u4 num;         // number of elements
        u1 type;        // element type
        u8 elements[];  // elements
    } *rec = ReadAs<PrimitiveArrayDump>();
    if (!rec) {
        m_curPos += sizeof(PrimitiveArrayDump);
        return;
    }

    ID id = SwapEndian(rec->id);
    m_arrays[id].type = BasicType(rec->type);
    m_arrays[id].cls = 0;
    m_arrays[id].num = SwapEndian(rec->num);
    if (verbose) {
        printf("[PRIMITIVE ARRAY DUMP@0x%zx] id = 0x%" PRIx64 ", num = %u, type = %u\n",
            m_curPos, id, m_arrays[id].num, m_arrays[id].type);
    }

    m_curPos += sizeof(PrimitiveArrayDump);
}

void Hprof::ParseHeapDumpRootUnknown(bool verbose)
{
    struct RootUnknown {
        u1 tag;
        ID objId;   // object ID
    } *rec = ReadAs<RootUnknown>();
    if (!rec) {
        m_curPos += sizeof(RootUnknown);
        return;
    }

    ID id = SwapEndian(rec->objId);
    m_unknown.insert(id);

    if (verbose) {
        printf("[ROOT UNKNOWN@0x%zx] id = 0x%" PRIx64 "\n", m_curPos, id);
    }

    m_curPos += sizeof(RootUnknown);
}

void Hprof::ParseCpuSamples(bool verbose)
{
    auto rec = ReadAs<CpuSamplesRecord>();
    if (rec == nullptr) {
        return;
    }
    auto period = SwapEndian(rec->period);
    auto num = SwapEndian(rec->num);
    auto &stackTraces = m_cpuSamples[period].stackTraces;

    for (u4 i = 0; i < num; i++) {
        stackTraces[SwapEndian(rec->samples[i].stackTraceIdx)] = SwapEndian(rec->samples[i].num);
    }

    if (verbose) {
        printf("[CPU SAMPLES@0x%zx] period = %u, samples = [", m_curPos, period);
        for (u4 i = 0; i < num; i++) {
            printf("[%u, %u]%s", SwapEndian(rec->samples[i].stackTraceIdx),
                SwapEndian(rec->samples[i].num), i < num - 1 ? ", " : "");
        }
        printf("]\n");
    }

    m_curPos += sizeof(RecordHeader) + SwapEndian(rec->length);
}

const std::string &Hprof::GenData()
{
    m_data = std::string(m_headerFlag, sizeof(m_headerFlag));
    WriteAs(SwapEndian((u4)sizeof(ID)));

    timeval time = {0};
    gettimeofday(&time, nullptr);
    u8 us = static_cast<u8>(time.tv_sec * 1000000 + time.tv_usec);
    WriteAs(SwapEndian(us));

    for (auto string : m_stringsMap) {
        GenString(string.second, string.first);
    }

    for (auto frame : m_framesMap) {
        GenStackFrame(frame.second, frame.first);
    }

    for (auto trace : m_stackTracesMap) {
        GenStackTrace(trace.second, trace.first);
    }

    for (auto sample : m_cpuSamples) {
        GenCpuSample(sample.first, sample.second);
    }

    return m_data;
}

void Hprof::PutCpuSample(const std::vector<std::string> &callChain, u4 period)
{
    StackTrace stackTrace = { .thread = 0 };
    for (auto func : callChain) {
        ID name = GetMapValue(m_stringsMap, func);
        Frame frame = { .name = name, .fileName = 0, .line = 0 };
        ID frameId = GetMapValue(m_framesMap, frame);
        stackTrace.frames.push_back(frameId);
    }

    m_cpuSamples[period].stackTraces[GetMapValue(m_stackTracesMap, stackTrace)]++;
}

void Hprof::GenString(ID id, const std::string &str)
{
    StringRecord rec;
    rec.tag = STRING;
    rec.length = SwapEndian((u8)(sizeof(StringRecord) + str.size() - sizeof(RecordHeader)));
    rec.id = SwapEndian(id);
    WriteAs(rec);
    m_data.append(str);
}

void Hprof::GenStackFrame(ID id, const Frame &frame)
{
    StackFrameRecord rec;
    rec.tag = STACK_FRAME;
    rec.length = SwapEndian((u8)(sizeof(StackFrameRecord)) - sizeof(RecordHeader));
    rec.id = SwapEndian(id);
    rec.name = SwapEndian(frame.name);
    rec.fileName = SwapEndian(frame.fileName);
    rec.line = SwapEndian(frame.line);
    WriteAs(rec);
}

void Hprof::GenStackTrace(u4 idx, const StackTrace &stackTrace)
{
    StackTraceRecord rec;
    rec.tag = STACK_TRACE;
    rec.length = SwapEndian(
        (u8)(sizeof(StackTraceRecord) + sizeof(ID) * stackTrace.frames.size()) - sizeof(RecordHeader));
    rec.idx = SwapEndian(idx);
    rec.thread = SwapEndian(stackTrace.thread);
    rec.frameNum = SwapEndian((u4)stackTrace.frames.size());
    WriteAs(rec);
    for (auto id : stackTrace.frames) {
        WriteAs(SwapEndian(id));
    }
}

void Hprof::GenCpuSample(u4 period, const Sample &sample)
{
    CpuSamplesRecord rec;
    rec.tag = CPU_SAMPLES;
    rec.length = SwapEndian(
        (u8)(sizeof(CpuSamplesRecord) + sizeof(rec.samples[0]) * sample.stackTraces.size()) - sizeof(RecordHeader));
    rec.period = SwapEndian(period);
    rec.num = SwapEndian((u4)sample.stackTraces.size());
    WriteAs(rec);
    for (auto it : sample.stackTraces) {
        WriteAs(SwapEndian(it.second));
        WriteAs(SwapEndian(it.first));
    }
}

#pragma pack(pop)