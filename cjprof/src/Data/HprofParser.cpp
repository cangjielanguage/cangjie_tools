// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <cinttypes>
#include "Data/HprofParser.h"

void HprofParserBase::SetData(const std::string &data)
{
    m_data = data;
    m_curPos = 0;
}

// ReadId and WriteId are based on idSize (from HprofData)
HprofParserBase::ID HprofParserBase::ReadId()
{
    if (m_curPos >= m_data.size() || m_data.size() - m_curPos < m_dataRef.idSize) {
        return 0;
    }
    if (m_dataRef.idSize == 4) {
        u4 val = *(u4 *)(m_data.data() + m_curPos);
        m_curPos += 4;
        return SwapEndian(val);
    } else if (m_dataRef.idSize == 8) {
        u8 val = *(u8 *)(m_data.data() + m_curPos);
        m_curPos += 8;
        return SwapEndian(val);
    }
    return 0;
}

void HprofParserBase::WriteId(ID id)
{
    if (m_dataRef.idSize == 4) {
        WriteAs(SwapEndian(static_cast<u4>(id)));
    } else if (m_dataRef.idSize == 8) {
        WriteAs(SwapEndian(static_cast<u8>(id)));
    }
}

// ============================================================================
// HprofParserV2 Implementation (Format 1.0.2)
// ============================================================================
// V2 Format: ID is dynamic (based on idSize from file header, currently u4)

// V2 STRING Format: u4 length, u4 id, u1 str[]
void HprofParserV2::ParseString(bool verbose)
{
    size_t startPos = m_curPos;
    u4 length = ReadAndSwap<u4>();
    if (length == 0 && m_curPos == startPos) {
        fprintf(stderr, "error: StringRecord length truncated at offset 0x%zx\n", startPos);
        m_curPos = m_data.size();
        return;
    }

    u4 id = ReadAndSwap<u4>();
    if (id == 0 && m_curPos == startPos + sizeof(u4)) {
        fprintf(stderr, "error: StringRecord id truncated at offset 0x%zx\n", startPos);
        m_curPos = m_data.size();
        return;
    }

    if (length < sizeof(u4)) {
        fprintf(stderr, "error: StringRecord length too small at offset 0x%zx\n", startPos);
        m_curPos = m_data.size();
        return;
    }
    size_t strLen = static_cast<size_t>(length - sizeof(u4));
    if (m_curPos + strLen > m_data.size()) {
        fprintf(stderr, "error: StringRecord data truncated at offset 0x%zx\n", startPos);
        m_curPos = m_data.size();
        return;
    }
    m_dataRef.strings[id] = std::string(m_data.data() + m_curPos, strLen);
    m_curPos += strLen;

    if (verbose) {
        printf("[STRING@0x%zx] id = 0x%x, str = \"%s\"\n", startPos, id, m_dataRef.strings[id].c_str());
    }
}

// V2 CLASS (LoadClass) Format: u4 id, u4 name, u4 size (no length field)
void HprofParserV2::ParseClass(bool verbose)
{
    size_t startPos = m_curPos;
    u4 id = ReadAndSwap<u4>();
    u4 name = ReadAndSwap<u4>();
    u4 size = ReadAndSwap<u4>();

    m_dataRef.classes[id].name = name;
    m_dataRef.classes[id].size = size;

    if (verbose) {
        printf("[LOAD CLASS@0x%zx] id = 0x%x, name id = 0x%" PRIx64 ", size = %u\n",
               startPos, id, m_dataRef.classes[id].name, m_dataRef.classes[id].size);
    }
}

// V2 STACK_FRAME Format: u8 length, u4 id, u4 name, u4 file, u4 line
void HprofParserV2::ParseStackFrame(bool verbose)
{
    size_t startPos = m_curPos;
    // Skip length field (u8)
    m_curPos += sizeof(u8);

    // V2: id/name/file are fixed u4 (regardless of idSize)
    u4 id = ReadAndSwap<u4>();
    u4 name = ReadAndSwap<u4>();
    u4 fileName = ReadAndSwap<u4>();
    u4 line = ReadAndSwap<u4>();

    m_dataRef.frames[id] = { name, fileName, line };

    if (verbose) {
        printf("[STACK FRAME@0x%zx] id = 0x%x, method name id = 0x%x, file name id = 0x%x, line = %u\n",
            startPos, id, name, fileName, line);
    }
}

// V2 STACK_TRACE Format: u8 length, u4 idx, u4 thread, u4 num, u4 frames[]
void HprofParserV2::ParseStackTrace(bool verbose)
{
    size_t startPos = m_curPos;
    // Skip length field (u8)
    m_curPos += sizeof(u8);

    u4 idx = ReadAndSwap<u4>();
    m_dataRef.stackTraces[idx].thread = ReadAndSwap<u4>();
    u4 frameNum = ReadAndSwap<u4>();
    constexpr u4 MAX_FRAME_NUM = 65536;
    if (frameNum > MAX_FRAME_NUM) {
        frameNum = MAX_FRAME_NUM;
    }

    // V2: frames are fixed u4 (regardless of idSize)
    for (size_t i = 0; i < frameNum; i++) {
        auto frameId = ReadAndSwap<u4>();
        if (frameId == 0) { break; }
        m_dataRef.stackTraces[idx].frames.push_back(frameId);
    }

    if (verbose) {
        printf("[STACK TRACE@0x%zx] idx = %u, thread idx = %u, frames = [",
               startPos, idx, m_dataRef.stackTraces[idx].thread);

        auto frames = m_dataRef.stackTraces[idx].frames;
        for (size_t i = 0; i < frames.size(); i++) {
            printf("0x%" PRIx64 "%s", frames[i], i < frames.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }
}

// V2 START_THREAD Format: u8 length, u4 idx, ID id, u4 stIdx, ID name
void HprofParserV2::ParseStartThread(bool verbose)
{
    size_t startPos = m_curPos;
    // Skip length field (u8)
    m_curPos += sizeof(u8);

    u4 idx = ReadAndSwap<u4>();
    // V2: id and name are dynamic ID (based on idSize)
    ID id = ReadId();
    u4 stackTraceIdx = ReadAndSwap<u4>();
    ID name = ReadId();

    m_dataRef.threads[id].idx = idx;
    m_dataRef.threads[id].stackTraceIdx = stackTraceIdx;
    m_dataRef.threads[id].name = name;

    if (verbose) {
        printf("[START THREAD@0x%zx] id = 0x%" PRIx64 ", thread idx = %u, stack trace idx = %u, name id = 0x%" PRIx64 "\n",
            startPos, id, idx, stackTraceIdx, name);
    }
}

// V2 CLASS_DUMP Format: u4 id, u4 name, u4 size
void HprofParserV2::ParseHeapDumpClassDump(bool verbose)
{
    size_t startPos = m_curPos;
    u4 id = ReadAndSwap<u4>();
    u4 name = ReadAndSwap<u4>();
    u4 size = ReadAndSwap<u4>();

    m_dataRef.classes[id].name = name;
    m_dataRef.classes[id].size = size;

    if (verbose) {
        printf("[CLASS DUMP@0x%zx] id = 0x%x, name id = 0x%" PRIx64 ", size = %u\n",
               startPos, id, m_dataRef.classes[id].name, m_dataRef.classes[id].size);
    }
}

// V2 INSTANCE_DUMP Format: ID id, u4 cls, u4 num, ID values[]
void HprofParserV2::ParseHeapDumpInstanceDump(bool verbose, ObjectCategory category)
{
    size_t startPos = m_curPos;
    ID id = ReadId();
    u4 cls = ReadAndSwap<u4>();
    u4 num = ReadAndSwap<u4>();

    // V2: values are dynamic ID
    ReadIdVector(m_dataRef.instances[id].fields, num);
    m_dataRef.instances[id].cls = cls;
    m_dataRef.objectCategories[id] = category;

    if (verbose) {
        printf("[INSTANCE DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%x, value = [", startPos, id, cls);

        auto fields = m_dataRef.instances[id].fields;
        for (size_t i = 0; i < fields.size(); i++) {
            printf("0x%" PRIx64 "%s", fields[i], i < fields.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }
}

// V2 OBJECT_ARRAY_DUMP Format: ID id, u4 num, u4 cls, ID elements[]
void HprofParserV2::ParseHeapDumpObjectArrayDump(bool verbose, ObjectCategory category)
{
    size_t startPos = m_curPos;
    ID id = ReadId();
    u4 num = ReadAndSwap<u4>();
    u4 cls = ReadAndSwap<u4>();

    m_dataRef.arrays[id].type = BasicType::OBJECT;
    m_dataRef.arrays[id].cls = cls;
    m_dataRef.arrays[id].num = num;
    m_dataRef.objectCategories[id] = category == ObjectCategory::INSTANCE_OBJECT ? ObjectCategory::OBJECT_ARRAY : category;
    ReadIdVector(m_dataRef.arrays[id].elements, num);

    if (verbose) {
        printf("[OBJECT ARRAY DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%x, elements = [", startPos, id, cls);

        auto elements = m_dataRef.arrays[id].elements;
        for (size_t i = 0; i < num; i++) {
            printf("0x%" PRIx64 "%s", elements[i], i < elements.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }
}

// V2 STRUCT_ARRAY_DUMP Format: ID id, u4 compNum, u4 num, u4 cls, ID elements[]
void HprofParserV2::ParseHeapDumpStructArrayDump(bool verbose, ObjectCategory category)
{
    size_t startPos = m_curPos;
    ID id = ReadId();
    m_dataRef.componentNums[id] = ReadAndSwap<u4>();
    u4 num = ReadAndSwap<u4>();
    u4 cls = ReadAndSwap<u4>();

    m_dataRef.arrays[id].type = BasicType::OBJECT;
    m_dataRef.arrays[id].cls = cls;
    m_dataRef.arrays[id].num = num;
    m_dataRef.objectCategories[id] = category == ObjectCategory::INSTANCE_OBJECT ? ObjectCategory::STRUCT_ARRAY : category;
    ReadIdVector(m_dataRef.arrays[id].elements, num);

    if (verbose) {
        printf("[STRUCT ARRAY DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%x, elements = [", startPos, id, cls);

        auto elements = m_dataRef.arrays[id].elements;
        for (size_t i = 0; i < num; i++) {
            printf("0x%" PRIx64 "%s", elements[i], i < elements.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }
}

// ============================================================================
// HprofParserV1 Implementation (Format 1.0.1)
// ============================================================================
// V1 Format: ID is fixed u8 (8 bytes)

// V1 STRING Format: u8 length, ID(u8) id, u1 str[]
void HprofParserV1::ParseString(bool verbose)
{
    size_t startPos = m_curPos;
    u8 length = ReadAndSwap<u8>();
    u8 id = ReadAndSwap<u8>();

    if (length < sizeof(u8)) {
        fprintf(stderr, "error: StringRecord length too small at offset 0x%zx\n", startPos);
        m_curPos = m_data.size();
        return;
    }
    size_t strLen = static_cast<size_t>(length - sizeof(u8));
    if (m_curPos + strLen > m_data.size()) {
        fprintf(stderr, "error: StringRecord data truncated at offset 0x%zx\n", startPos);
        m_curPos = m_data.size();
        return;
    }
    m_dataRef.strings[id] = std::string(m_data.data() + m_curPos, strLen);
    m_curPos += strLen;

    if (verbose) {
        printf("[STRING@0x%zx] id = 0x%" PRIx64 ", str = \"%s\"\n", startPos, id, m_dataRef.strings[id].c_str());
    }
}

// V1 CLASS (LoadClass) Format: u8 length, ID(u8) id, ID(u8) name (no size)
void HprofParserV1::ParseClass(bool verbose)
{
    size_t startPos = m_curPos;
    // Skip length field
    m_curPos += sizeof(u8);

    u8 id = ReadAndSwap<u8>();
    u8 name = ReadAndSwap<u8>();

    m_dataRef.classes[id].name = name;

    if (verbose) {
        printf("[LOAD CLASS@0x%zx] id = 0x%" PRIx64 ", name id = 0x%" PRIx64 "\n", startPos, id, m_dataRef.classes[id].name);
    }
}

// V1 STACK_FRAME Format: u8 length, u8 id, u8 name, u8 file, u4 line
void HprofParserV1::ParseStackFrame(bool verbose)
{
    size_t startPos = m_curPos;
    // Skip length field (u8)
    m_curPos += sizeof(u8);

    // V1: id/name/file are fixed u8
    u8 id = ReadAndSwap<u8>();
    u8 name = ReadAndSwap<u8>();
    u8 fileName = ReadAndSwap<u8>();
    u4 line = ReadAndSwap<u4>();

    m_dataRef.frames[id] = { name, fileName, line };

    if (verbose) {
        printf("[STACK FRAME@0x%zx] id = 0x%" PRIx64 ", method name id = 0x%" PRIx64 ", file name id = 0x%" PRIx64 ", line = %u\n",
            startPos, id, name, fileName, line);
    }
}

// V1 STACK_TRACE Format: u8 length, u4 idx, u4 thread, u4 num, u8 frames[]
void HprofParserV1::ParseStackTrace(bool verbose)
{
    size_t startPos = m_curPos;
    // Skip length field (u8)
    m_curPos += sizeof(u8);

    u4 idx = ReadAndSwap<u4>();
    m_dataRef.stackTraces[idx].thread = ReadAndSwap<u4>();
    u4 frameNum = ReadAndSwap<u4>();
    constexpr u4 MAX_FRAME_NUM = 65536;
    if (frameNum > MAX_FRAME_NUM) {
        frameNum = MAX_FRAME_NUM;
    }

    // V1: frames are fixed u8
    for (size_t i = 0; i < frameNum; i++) {
        m_dataRef.stackTraces[idx].frames.push_back(ReadAndSwap<u8>());
    }

    if (verbose) {
        printf("[STACK TRACE@0x%zx] idx = %u, thread idx = %u, frames = [",
               startPos, idx, m_dataRef.stackTraces[idx].thread);

        auto frames = m_dataRef.stackTraces[idx].frames;
        for (size_t i = 0; i < frames.size(); i++) {
            printf("0x%" PRIx64 "%s", frames[i], i < frames.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }
}

// V1 START_THREAD Format: u8 length, u4 idx, u8 id, u4 stIdx, u8 name
void HprofParserV1::ParseStartThread(bool verbose)
{
    size_t startPos = m_curPos;
    // Skip length field (u8)
    m_curPos += sizeof(u8);

    u4 idx = ReadAndSwap<u4>();
    // V1: id and name are fixed u8
    u8 id = ReadAndSwap<u8>();
    u4 stackTraceIdx = ReadAndSwap<u4>();
    u8 name = ReadAndSwap<u8>();

    m_dataRef.threads[id].idx = idx;
    m_dataRef.threads[id].stackTraceIdx = stackTraceIdx;
    m_dataRef.threads[id].name = name;

    if (verbose) {
        printf("[START THREAD@0x%zx] id = 0x%" PRIx64 ", thread idx = %u, stack trace idx = %u, name id = 0x%" PRIx64 "\n",
            startPos, id, idx, stackTraceIdx, name);
    }
}

// V1 CLASS_DUMP Format: ID(u8) id, u4 size (no name)
void HprofParserV1::ParseHeapDumpClassDump(bool verbose)
{
    size_t startPos = m_curPos;
    u8 id = ReadAndSwap<u8>();
    u4 size = ReadAndSwap<u4>();

    m_dataRef.classes[id].size = size;

    if (verbose) {
        printf("[CLASS DUMP@0x%zx] id = 0x%" PRIx64 ", size = %u\n", startPos, id, m_dataRef.classes[id].size);
    }
}

// V1 INSTANCE_DUMP Format: ID(u8) id, ID(u8) cls, u4 num, ID(u8) values[]
void HprofParserV1::ParseHeapDumpInstanceDump(bool verbose, ObjectCategory category)
{
    size_t startPos = m_curPos;
    u8 id = ReadAndSwap<u8>();
    u8 cls = ReadAndSwap<u8>();
    u4 num = ReadAndSwap<u4>();

    // V1: values are fixed u8
    for (size_t i = 0; i < num; i++) {
        m_dataRef.instances[id].fields.push_back(ReadAndSwap<u8>());
    }
    m_dataRef.instances[id].cls = cls;
    m_dataRef.objectCategories[id] = category;

    if (verbose) {
        printf("[INSTANCE DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%" PRIx64 ", value = [", startPos, id, cls);

        auto fields = m_dataRef.instances[id].fields;
        for (size_t i = 0; i < fields.size(); i++) {
            printf("0x%" PRIx64 "%s", fields[i], i < fields.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }
}

// V1 OBJECT_ARRAY_DUMP Format: ID(u8) id, u4 num, ID(u8) cls, ID(u8) elements[]
void HprofParserV1::ParseHeapDumpObjectArrayDump(bool verbose, ObjectCategory category)
{
    size_t startPos = m_curPos;
    u8 id = ReadAndSwap<u8>();
    u4 num = ReadAndSwap<u4>();
    u8 cls = ReadAndSwap<u8>();

    m_dataRef.arrays[id].type = BasicType::OBJECT;
    m_dataRef.arrays[id].cls = cls;
    m_dataRef.arrays[id].num = num;
    m_dataRef.objectCategories[id] = category == ObjectCategory::INSTANCE_OBJECT ? ObjectCategory::OBJECT_ARRAY : category;
    for (size_t i = 0; i < num; i++) {
        m_dataRef.arrays[id].elements.push_back(ReadAndSwap<u8>());
    }

    if (verbose) {
        printf("[OBJECT ARRAY DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%" PRIx64 ", elements = [", startPos, id, cls);

        auto elements = m_dataRef.arrays[id].elements;
        for (size_t i = 0; i < num; i++) {
            printf("0x%" PRIx64 "%s", elements[i], i < elements.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }
}

// V1 STRUCT_ARRAY_DUMP Format: ID(u8) id, u4 compNum, u4 num, ID(u8) cls, ID(u8) elements[]
void HprofParserV1::ParseHeapDumpStructArrayDump(bool verbose, ObjectCategory category)
{
    size_t startPos = m_curPos;
    u8 id = ReadAndSwap<u8>();
    m_dataRef.componentNums[id] = ReadAndSwap<u4>();
    u4 num = ReadAndSwap<u4>();
    u8 cls = ReadAndSwap<u8>();

    m_dataRef.arrays[id].type = BasicType::OBJECT;
    m_dataRef.arrays[id].cls = cls;
    m_dataRef.arrays[id].num = num;
    m_dataRef.objectCategories[id] = category == ObjectCategory::INSTANCE_OBJECT ? ObjectCategory::STRUCT_ARRAY : category;
    for (size_t i = 0; i < num; i++) {
        m_dataRef.arrays[id].elements.push_back(ReadAndSwap<u8>());
    }

    if (verbose) {
        printf("[STRUCT ARRAY DUMP@0x%zx] id = 0x%" PRIx64 ", class = 0x%" PRIx64 ", elements = [", startPos, id, cls);

        auto elements = m_dataRef.arrays[id].elements;
        for (size_t i = 0; i < num; i++) {
            printf("0x%" PRIx64 "%s", elements[i], i < elements.size() - 1 ? ", " : "");
        }

        printf("]\n");
    }
}