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

// Static empty containers for HprofData
const std::map<HprofData::ID, std::string> HprofData::emptyStrings;
const std::map<HprofData::ID, HprofData::Frame> HprofData::emptyFrames;
const std::map<HprofData::u4, HprofData::StackTrace> HprofData::emptyStackTraces;
const std::map<HprofData::ID, HprofData::Thread> HprofData::emptyThreads;
const std::unordered_map<HprofData::ID, HprofData::Class> HprofData::emptyClasses;
const std::map<HprofData::ID, HprofData::Instance> HprofData::emptyInstances;
const std::unordered_map<HprofData::ID, HprofData::Array> HprofData::emptyArrays;
const std::unordered_map<HprofData::ID, HprofData::Local> HprofData::emptyLocals;
const std::unordered_set<HprofData::ID> HprofData::emptyGlobals;
const std::unordered_set<HprofData::ID> HprofData::emptyUnknown;
const std::map<HprofData::u4, HprofData::Sample> HprofData::emptyCpuSamples;
const std::unordered_map<HprofData::ID, HprofData::u4> HprofData::emptyComponentNums;
const std::unordered_map<HprofData::ID, ObjectCategory> HprofData::emptyObjectCategories;

#pragma pack(push, 1)

bool Hprof::Parse(const std::string &data, bool verbose)
{
    // Read file header to determine format version
    if (data.size() < sizeof(FileHeader)) {
        fprintf(stderr, "error: Invalid file contents or file format.\n");
        return false;
    }

    auto header = (FileHeader *)(data.data());
    std::string ident(header->ident);

    // Create data storage first
    m_data = std::make_unique<HprofData>();

    // Strategy pattern: create appropriate parser based on format version
    if (ident == m_headerFlagV2) {
        m_parser = std::make_unique<HprofParserV2>(*m_data);
        // V2 format: ID size is dynamic, read from file header (4 or 8)
        m_data->idSize = HprofParserBase::SwapEndian(header->idSize);
    } else if (ident == m_headerFlagV1) {
        m_parser = std::make_unique<HprofParserV1>(*m_data);
        // V1 format: ID size is fixed to 8 bytes (set in HprofParserV1 constructor)
    } else {
        fprintf(stderr, "error: Invalid file contents or file format.\n");
        return false;
    }

    // Initialize parser with data
    m_parser->SetData(data);
    m_data->fileTime = header->timeHigh;
    m_data->fileTime = (m_data->fileTime << 32) + header->timeLow;
    m_parser->SetCurPos(sizeof(FileHeader));

    // Main parsing loop using strategy pattern - delegate to parser
    std::map<Tag, std::function<void(bool)>> recordParsers = {
        { STACK_FRAME, std::bind(&HprofParserBase::ParseStackFrame, m_parser.get(), std::placeholders::_1) },
        { STACK_TRACE, std::bind(&HprofParserBase::ParseStackTrace, m_parser.get(), std::placeholders::_1) },
        { START_THREAD, std::bind(&HprofParserBase::ParseStartThread, m_parser.get(), std::placeholders::_1) },
        { HEAP_DUMP, std::bind(&Hprof::ParseHeapDump, this, std::placeholders::_1) },
        { CPU_SAMPLES, std::bind(&Hprof::ParseCpuSamples, this, std::placeholders::_1) }
    };

    while (m_parser->GetCurPos() < data.size()) {
        auto tag = m_parser->ReadAs<Tag>();
        if (!tag) {
            fprintf(stderr, "error: Record was truncated at offset 0x%zx\n", m_parser->GetCurPos());
            return false;
        }
        m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(Tag));

        // CLASS record has no length field, delegate to parser
        if (*tag == CLASS) {
            m_parser->ParseClass(verbose);
            continue;
        }

        // STRING record has u4/u8 length, delegate to parser
        if (*tag == STRING) {
            m_parser->ParseString(verbose);
            continue;
        }

        // For other records, check if there's a parser
        auto it = recordParsers.find(*tag);
        if (it != recordParsers.end()) {
            it->second(verbose);
            continue;
        }

        // Unknown tag: read length (u8) and skip
        auto lengthRec = m_parser->ReadAs<u8>();
        if (!lengthRec) {
            fprintf(stderr, "error: Record length truncated at offset 0x%zx\n", m_parser->GetCurPos());
            return false;
        }
        m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u8));
        auto size = HprofParserBase::SwapEndian(*lengthRec);
        if (m_parser->GetCurPos() + size > data.size()) {
            fprintf(stderr, "error: Record length is too long at offset 0x%zx\n", m_parser->GetCurPos());
            return false;
        }

        if (verbose) {
            printf("[SKIPED@0x%zx] tag = %u\n", m_parser->GetCurPos() - sizeof(Tag) - sizeof(u8), *tag);
        }

        m_parser->SetCurPos(m_parser->GetCurPos() + size);
    }

    if (m_parser->GetCurPos() != data.size()) {
        fprintf(stderr, "error: Unknown record or the length of the last record is too long.\n");
        return false;
    }

    return true;
}

void Hprof::ParseHeapDump(bool verbose)
{
    auto lengthRec = m_parser->ReadAs<u8>();
    if (lengthRec == nullptr) {
        return;
    }
    m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u8));
    auto length = HprofParserBase::SwapEndian(*lengthRec);
    auto endPos = m_parser->GetCurPos() + length;

    std::unordered_map<HeapDumpSubTag, std::function<void(bool)>> subRecordParsers = {
        { ROOT_GLOBAL, std::bind(&Hprof::ParseHeapDumpRootGlobal, this, std::placeholders::_1) },
        { ROOT_LOCAL, std::bind(&Hprof::ParseHeapDumpRootLocal, this, std::placeholders::_1) },
        { CLASS_DUMP, std::bind(&Hprof::ParseHeapDumpClassDump, this, std::placeholders::_1) },
        { INSTANCE_DUMP, [this](bool v){ ParseHeapDumpInstanceDump(v, ObjectCategory::INSTANCE_OBJECT); } },
        { OBJECT_ARRAY_DUMP, [this](bool v){ ParseHeapDumpObjectArrayDump(v, ObjectCategory::INSTANCE_OBJECT); } },
        { PRIMITIVE_ARRAY_DUMP, [this](bool v){ ParseHeapDumpPrimitiveArrayDump(v, ObjectCategory::INSTANCE_OBJECT); } },
        { STRUCT_ARRAY_DUMP, [this](bool v){ ParseHeapDumpStructArrayDump(v, ObjectCategory::INSTANCE_OBJECT); } },
        { PINNED_INSTANCE_DUMP, [this](bool v){ ParseHeapDumpInstanceDump(v, ObjectCategory::PINNED_OBJECT); } },
        { LARGE_INSTANCE_DUMP, [this](bool v){ ParseHeapDumpInstanceDump(v, ObjectCategory::LARGE_OBJECT); } },
        { LARGE_OBJECT_ARRAY_DUMP, [this](bool v){ ParseHeapDumpObjectArrayDump(v, ObjectCategory::LARGE_OBJECT); } },
        { LARGE_PRIMITIVE_ARRAY_DUMP, [this](bool v){ ParseHeapDumpPrimitiveArrayDump(v, ObjectCategory::LARGE_OBJECT); } },
        { LARGE_STRUCT_ARRAY_DUMP, [this](bool v){ ParseHeapDumpStructArrayDump(v, ObjectCategory::LARGE_OBJECT); } },
        { UNMOVABLE_INSTANCE_DUMP, [this](bool v){ ParseHeapDumpInstanceDump(v, ObjectCategory::UNMOVABLE_OBJECT); } },
        { UNMOVABLE_OBJECT_ARRAY_DUMP, [this](bool v){ ParseHeapDumpObjectArrayDump(v, ObjectCategory::UNMOVABLE_OBJECT); } },
        { UNMOVABLE_PRIMITIVE_ARRAY_DUMP, [this](bool v){ ParseHeapDumpPrimitiveArrayDump(v, ObjectCategory::UNMOVABLE_OBJECT); } },
        { UNMOVABLE_STRUCT_ARRAY_DUMP, [this](bool v){ ParseHeapDumpStructArrayDump(v, ObjectCategory::UNMOVABLE_OBJECT); } },
        { ROOT_UNKNOWN, std::bind(&Hprof::ParseHeapDumpRootUnknown, this, std::placeholders::_1) }
    };

    while (m_parser->GetCurPos() < endPos) {
        auto tag = m_parser->ReadAs<HeapDumpSubTag>();
        if (tag == nullptr) {
            return;
        }
        m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(HeapDumpSubTag));
        auto it = subRecordParsers.find(*tag);
        if (it != subRecordParsers.end()) {
            it->second(verbose);
        } else {
            fprintf(stderr, "error: Unknown sub-tag %u at offset 0x%zx\n",
                    *tag, m_parser->GetCurPos() - sizeof(HeapDumpSubTag));
            return;
        }
    }
}

void Hprof::ParseHeapDumpRootGlobal(bool verbose)
{
    size_t startPos = m_parser->GetCurPos() - sizeof(u1);
    ID id = m_parser->ReadId();
    m_data->globals.insert(id);

    if (verbose) {
        printf("[ROOT GLOBAL@0x%zx] id = 0x%" PRIx64 "\n", startPos, id);
    }
}

void Hprof::ParseHeapDumpRootLocal(bool verbose)
{
    size_t startPos = m_parser->GetCurPos() - sizeof(u1);
    ID id = m_parser->ReadId();

    auto threadRec = m_parser->ReadAs<u4>();
    if (threadRec == nullptr) {
        return;
    }
    u4 thread = HprofParserBase::SwapEndian(*threadRec);
    m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u4));

    auto frameRec = m_parser->ReadAs<u4>();
    if (frameRec == nullptr) {
        return;
    }
    u4 frame = HprofParserBase::SwapEndian(*frameRec);
    m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u4));

    m_data->locals[id] = { thread, frame };

    if (verbose) {
        printf("[ROOT LOCAL@0x%zx] id = 0x%" PRIx64 ", thread idx = %u, frame = %u\n",
            startPos, id, m_data->locals[id].thread, m_data->locals[id].frame);
    }
}

void Hprof::ParseHeapDumpClassDump(bool verbose)
{
    // Delegate to parser strategy
    m_parser->ParseHeapDumpClassDump(verbose);
}

void Hprof::ParseHeapDumpInstanceDump(bool verbose, ObjectCategory category)
{
    // Delegate to parser strategy with category tracking
    m_parser->ParseHeapDumpInstanceDump(verbose, category);
}

void Hprof::ParseHeapDumpObjectArrayDump(bool verbose, ObjectCategory category)
{
    // Delegate to parser strategy with category tracking
    m_parser->ParseHeapDumpObjectArrayDump(verbose, category);
}

void Hprof::ParseHeapDumpStructArrayDump(bool verbose, ObjectCategory category)
{
    // Delegate to parser strategy with category tracking
    m_parser->ParseHeapDumpStructArrayDump(verbose, category);
}

void Hprof::ParseHeapDumpPrimitiveArrayDump(bool verbose, ObjectCategory category)
{
    size_t startPos = m_parser->GetCurPos() - sizeof(u1);
    ID id = m_parser->ReadId();

    auto numRec = m_parser->ReadAs<u4>();
    if (numRec == nullptr) {
        return;
    }
    m_data->arrays[id].num = HprofParserBase::SwapEndian(*numRec);
    m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u4));

    auto typeRec = m_parser->ReadAs<u1>();
    if (typeRec == nullptr) {
        return;
    }
    m_data->arrays[id].type = BasicType(*typeRec);
    m_data->arrays[id].cls = 0;
    m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u1));

m_data->objectCategories[id] = category == ObjectCategory::INSTANCE_OBJECT ? ObjectCategory::PRIMITIVE_ARRAY : category;
    if (verbose) {
        printf("[PRIMITIVE ARRAY DUMP@0x%zx] id = 0x%" PRIx64 ", num = %u, type = %u\n",
            startPos, id, m_data->arrays[id].num, m_data->arrays[id].type);
    }
}

void Hprof::ParseHeapDumpRootUnknown(bool verbose)
{
    size_t startPos = m_parser->GetCurPos() - sizeof(u1);
    ID id = m_parser->ReadId();
    m_data->unknown.insert(id);

    if (verbose) {
        printf("[ROOT UNKNOWN@0x%zx] id = 0x%" PRIx64 "\n", startPos, id);
    }
}

void Hprof::ParseCpuSamples(bool verbose)
{
    size_t startPos = m_parser->GetCurPos() - sizeof(Tag);
    auto lengthRec = m_parser->ReadAs<u8>();
    if (lengthRec == nullptr) {
        return;
    }
    m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u8));
    auto length = HprofParserBase::SwapEndian(*lengthRec);

    auto periodRec = m_parser->ReadAs<u4>();
    if (periodRec == nullptr) {
        return;
    }
    auto period = HprofParserBase::SwapEndian(*periodRec);
    m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u4));

    auto numRec = m_parser->ReadAs<u4>();
    if (numRec == nullptr) {
        return;
    }
    auto num = HprofParserBase::SwapEndian(*numRec);
    m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u4));

    auto &stackTraces = m_data->cpuSamples[period].stackTraces;

    for (u4 i = 0; i < num; i++) {
        auto sampleNumRec = m_parser->ReadAs<u4>();
        if (sampleNumRec == nullptr) {
            return;
        }
        auto sampleNum = HprofParserBase::SwapEndian(*sampleNumRec);
        m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u4));

        auto stackTraceIdxRec = m_parser->ReadAs<u4>();
        if (stackTraceIdxRec == nullptr) {
            return;
        }
        auto stackTraceIdx = HprofParserBase::SwapEndian(*stackTraceIdxRec);
        m_parser->SetCurPos(m_parser->GetCurPos() + sizeof(u4));

        stackTraces[stackTraceIdx] = sampleNum;
    }

    if (verbose) {
        printf("[CPU SAMPLES@0x%zx] period = %u, samples = [", startPos, period);
        for (auto it : stackTraces) {
            printf("[%u, %u]", it.first, it.second);
        }
        printf("]\n");
    }
}

const std::string &Hprof::GenData()
{
    // Generate V2 format by default
    m_outputData = std::string(m_headerFlagV2, sizeof(m_headerFlagV2));
    u4 idSize = HprofParserBase::SwapEndian(static_cast<u4>(sizeof(u4)));
    m_outputData.append(std::string((const char *)&idSize, sizeof(u4)));

    timeval time = {0};
    gettimeofday(&time, nullptr);
    u8 us = static_cast<u8>(time.tv_sec * 1000000 + time.tv_usec);
    u8 swappedUs = HprofParserBase::SwapEndian(us);
    m_outputData.append(std::string((const char *)&swappedUs, sizeof(u8)));

    for (auto string : m_stringsMap) {
        GenString(string.second, string.first);
    }

    for (auto frame : m_framesMap) {
        GenStackFrame(frame.second, frame.first);
    }

    for (auto trace : m_stackTracesMap) {
        GenStackTrace(trace.second, trace.first);
    }

    if (m_data) {
        for (auto sample : m_data->cpuSamples) {
            GenCpuSample(sample.first, sample.second);
        }
    }

    return m_outputData;
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

    if (!m_data) {
        m_data = std::make_unique<HprofData>();
    }
    m_data->cpuSamples[period].stackTraces[GetMapValue(m_stackTracesMap, stackTrace)]++;
}

void Hprof::GenString(ID id, const std::string &str)
{
    m_outputData.push_back(STRING);
    u4 length = sizeof(u4) + str.size();
    u4 swappedLength = HprofParserBase::SwapEndian(length);
    m_outputData.append(std::string((const char *)&swappedLength, sizeof(u4)));
    u4 swappedId = HprofParserBase::SwapEndian(static_cast<u4>(id));
    m_outputData.append(std::string((const char *)&swappedId, sizeof(u4)));
    m_outputData.append(str);
}

void Hprof::GenStackFrame(ID id, const Frame &frame)
{
    m_outputData.push_back(STACK_FRAME);
    u8 length = sizeof(u4) * 4;  // id + name + fileName + line (V2 format uses u4)
    u8 swappedLength = HprofParserBase::SwapEndian(length);
    m_outputData.append(std::string((const char *)&swappedLength, sizeof(u8)));

    u4 swappedId = HprofParserBase::SwapEndian(static_cast<u4>(id));
    u4 swappedName = HprofParserBase::SwapEndian(static_cast<u4>(frame.name));
    u4 swappedFileName = HprofParserBase::SwapEndian(static_cast<u4>(frame.fileName));
    u4 swappedLine = HprofParserBase::SwapEndian(frame.line);

    m_outputData.append(std::string((const char *)&swappedId, sizeof(u4)));
    m_outputData.append(std::string((const char *)&swappedName, sizeof(u4)));
    m_outputData.append(std::string((const char *)&swappedFileName, sizeof(u4)));
    m_outputData.append(std::string((const char *)&swappedLine, sizeof(u4)));
}

void Hprof::GenStackTrace(u4 idx, const StackTrace &stackTrace)
{
    m_outputData.push_back(STACK_TRACE);
    u8 length = sizeof(u4) * 3 + sizeof(u4) * stackTrace.frames.size();
    u8 swappedLength = HprofParserBase::SwapEndian(length);
    m_outputData.append(std::string((const char *)&swappedLength, sizeof(u8)));

    u4 swappedIdx = HprofParserBase::SwapEndian(idx);
    u4 swappedThread = HprofParserBase::SwapEndian(stackTrace.thread);
    u4 swappedFrameNum = HprofParserBase::SwapEndian(static_cast<u4>(stackTrace.frames.size()));

    m_outputData.append(std::string((const char *)&swappedIdx, sizeof(u4)));
    m_outputData.append(std::string((const char *)&swappedThread, sizeof(u4)));
    m_outputData.append(std::string((const char *)&swappedFrameNum, sizeof(u4)));

    for (auto frameId : stackTrace.frames) {
        u4 swappedFrameId = HprofParserBase::SwapEndian(static_cast<u4>(frameId));
        m_outputData.append(std::string((const char *)&swappedFrameId, sizeof(u4)));
    }
}

void Hprof::GenCpuSample(u4 period, const Sample &sample)
{
    m_outputData.push_back(CPU_SAMPLES);
    u8 length = sizeof(u4) * 2 + sizeof(u4) * 2 * sample.stackTraces.size();
    u8 swappedLength = HprofParserBase::SwapEndian(length);
    m_outputData.append(std::string((const char *)&swappedLength, sizeof(u8)));

    u4 swappedPeriod = HprofParserBase::SwapEndian(period);
    u4 swappedNum = HprofParserBase::SwapEndian(static_cast<u4>(sample.stackTraces.size()));

    m_outputData.append(std::string((const char *)&swappedPeriod, sizeof(u4)));
    m_outputData.append(std::string((const char *)&swappedNum, sizeof(u4)));

    for (auto it : sample.stackTraces) {
        u4 swappedSampleNum = HprofParserBase::SwapEndian(it.second);
        u4 swappedStackTraceIdx = HprofParserBase::SwapEndian(it.first);
        m_outputData.append(std::string((const char *)&swappedSampleNum, sizeof(u4)));
        m_outputData.append(std::string((const char *)&swappedStackTraceIdx, sizeof(u4)));
    }
}

#pragma pack(pop)