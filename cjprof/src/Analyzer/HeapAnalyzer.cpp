// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <set>
#include <queue>
#include <iostream>
#include <chrono>
#include <iomanip>
#include "Analyzer/HeapAnalyzer.h"
#include "Analyzer/Logger.h"

static std::vector<std::pair<std::string, uint64_t>> g_phaseBreakdown;

static std::string GetStringSafe(const std::map<Hprof::ID, std::string>& strings, Hprof::ID id)
{
    auto it = strings.find(id);
    return it != strings.end() ? it->second : "";
}

static void AddPhase(const std::string& name, uint64_t ms)
{
    g_phaseBreakdown.emplace_back(name, ms);
}

static void PrintPhaseBreakdown(uint64_t totalMs)
{
    LOG_INFO("[perf] ============================================");
    LOG_INFO("[perf] Phase Breakdown:");
    uint64_t sum = 0;
    for (const auto& p : g_phaseBreakdown) {
        std::ostringstream oss;
        oss << "[perf]   " << std::setw(40) << std::right << p.first << ": " << std::setw(10) << std::right << p.second << " ms";
        LOG_INFO("{}", oss.str());
        sum += p.second;
    }
    {
        std::ostringstream oss;
        oss << "[perf]   " << std::setw(40) << std::right << "SUM" << ": " << std::setw(10) << std::right << sum << " ms";
        LOG_INFO("{}", oss.str());
    }
    {
        std::ostringstream oss;
        oss << "[perf]   " << std::setw(40) << std::right << "TOTAL" << ": " << std::setw(10) << std::right << totalMs << " ms";
        LOG_INFO("{}", oss.str());
    }
    if (totalMs > sum) {
        std::ostringstream oss;
        oss << "[perf]   " << std::setw(40) << std::right << "UNACCOUNTED" << ": " << std::setw(10) << std::right << (totalMs - sum) << " ms";
        LOG_INFO("{}", oss.str());
    }
    LOG_INFO("[perf] ============================================");
    g_phaseBreakdown.clear();
}

bool HeapAnalyzer::SetData(const std::string &file)
{
    auto t0 = std::chrono::steady_clock::now();

    std::ifstream ifs(file, std::ifstream::binary);
    if (ifs.fail()) {
        fprintf(stderr, "error: Open file '%s' failed.\n", file.c_str());
        return false;
    }

    ifs.seekg(0, ifs.end);
    auto size = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    size_t buf_size = static_cast<size_t>(size);
    std::vector<char> buf(buf_size);
    ifs.read(buf.data(), buf_size);
    m_data = std::string(buf.data(), buf_size);
    m_fileSize = buf_size;
    m_filePath = file;

    auto t1 = std::chrono::steady_clock::now();
    auto setDataMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    LOG_INFO("[perf] SetData (file read): {} ms", setDataMs);
    AddPhase("SetData(file read)", setDataMs);
    return true;
}

bool HeapAnalyzer::Analyze(bool verbose)
{
    auto t0 = std::chrono::steady_clock::now();
    if (!m_hprof.Parse(m_data, verbose)) {
        return false;
    }
    auto t1 = std::chrono::steady_clock::now();
    if (!m_dumpReport) {
        AnalyzeObject();
    }
    auto t2 = std::chrono::steady_clock::now();
    if (!m_dumpReport) {
        AnalyzeThread();
        FilterPlaceholderObjects();
    }
    auto t3 = std::chrono::steady_clock::now();

    auto parseMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto analyzeObjMs = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    auto analyzeThreadMs = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    LOG_INFO("[perf] Hprof::Parse: {} ms", parseMs);
    LOG_INFO("[perf] AnalyzeObject: {} ms", analyzeObjMs);
    LOG_INFO("[perf] AnalyzeThread: {} ms", analyzeThreadMs);
    AddPhase("Hprof::Parse", parseMs);
    // AnalyzeObject breakdown is added by its internal sub-phases (AnalyzeInstance + AnalyzeArray + ...)
    AddPhase("AnalyzeThread", analyzeThreadMs);
    return true;
}

void HeapAnalyzer::ShowThread()
{
    auto t0 = std::chrono::steady_clock::now();
    printf("Object/Stack Frame                                                            "
        "Shallow Heap   Retained Heap\n");
    printf("============================================================================  "
        "=============  =============\n");
    for (auto thread : m_threads) {
        printf("%s\n", thread.name.c_str());
        for (auto frame : thread.frames) {
            if (frame.fileName.empty()) {
                printf("  at %s\n", frame.name.c_str());
            } else {
                printf("  at %s (%s:%d)\n", frame.name.c_str(), frame.fileName.c_str(), frame.line);
            }
            for (auto local : frame.locals) {
                std::stringstream ss;
                ss << local->name << " @ 0x" << std::hex << local->id;
                printf("    <local> %-64s  %13llu  %13llu\n", ss.str().c_str(),
                    static_cast<unsigned long long>(local->size),
                    static_cast<unsigned long long>(local->retainedSize));
            }
        }
    }
    printf("\n");
    auto t1 = std::chrono::steady_clock::now();
    auto showThreadMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    LOG_INFO("[perf] ShowThread: {} ms", showThreadMs);
    AddPhase("ShowThread", showThreadMs);
}

void HeapAnalyzer::ShowObject()
{
    auto t0 = std::chrono::steady_clock::now();
    struct ObjectInfo {
        unsigned count = 0;
        uint64_t size = 0;
        uint64_t retainedSize = 0;
    };
    auto selfRefChecker = [&](std::shared_ptr<Object> obj) {
        auto tmp = obj;
        std::unordered_set<std::shared_ptr<Object>> memo;
        while ((tmp->inRef.size() == 1) && (memo.find(tmp) == memo.end())) {
            memo.emplace(tmp);
            tmp = GetObject(*tmp->inRef.begin());
            if (tmp->name == obj->name) {
                return true;
            }
        }

        return false;
    };
    std::map<std::string, ObjectInfo> objInfos;
    for (auto obj : m_objects) {
        objInfos[obj->name].count++;
        objInfos[obj->name].size += obj->size;
        if (!selfRefChecker(obj) || (objInfos[obj->name].retainedSize == 0)) {
            objInfos[obj->name].retainedSize += obj->retainedSize;
        }
    }

    std::vector<std::pair<std::string, ObjectInfo>> sortedObjInfos(objInfos.begin(), objInfos.end());
    auto comp = [](const std::pair<std::string, ObjectInfo> &a, const std::pair<std::string, ObjectInfo> &b) {
        return a.second.retainedSize == b.second.retainedSize ?
            a.second.size == b.second.size ? a.first < b.first : a.second.size > b.second.size : a.second.retainedSize > b.second.retainedSize;
    };
    std::sort(sortedObjInfos.begin(), sortedObjInfos.end(), comp);

    printf("Object Type                                                       "
        "Objects        Shallow Heap   Retained Heap\n");
    printf("================================================================  "
        "=============  =============  =============\n");
    for (auto obj : sortedObjInfos) {
        if (obj.first.empty()) {
            /* not in heap, skip */
            continue;
        }

        printf("%-64s  %13u  %13llu  %13llu\n", obj.first.c_str(), obj.second.count,
            static_cast<unsigned long long>(obj.second.size),
            static_cast<unsigned long long>(obj.second.retainedSize));
    }
    printf("\n");
    auto t1 = std::chrono::steady_clock::now();
    auto showObjectMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    LOG_INFO("[perf] ShowObject: {} ms", showObjectMs);
    AddPhase("ShowObject", showObjectMs);
}

void HeapAnalyzer::ShowReference(const std::string &objNameList, int maxDepth, bool incoming)
{
    auto t0 = std::chrono::steady_clock::now();
    std::string str = objNameList;
    std::unordered_set<std::string> objNames;
    /* Separate names by ';'. */
    auto pos = str.find(';');
    while (pos != std::string::npos) {
        objNames.emplace(str.substr(0, pos));
        str = str.substr(pos + 1);
        pos = str.find(';');
    }

    if (!str.empty()) {
        objNames.emplace(str);
    }

    if (!objNames.empty()) {
        std::unordered_set<std::string> matchedNames;
        for (auto obj : m_objects) {
            if (objNames.count(obj->name) != 0) {
                matchedNames.insert(obj->name);
            }
        }
        for (const auto& name : objNames) {
            if (matchedNames.find(name) == matchedNames.end()) {
                printf("No objects found matching: %s\n", name.c_str());
            }
        }
    }

    const auto& locals = m_hprof.GetLocalsRoots();
    const auto& globals = m_hprof.GetGlobalsRoots();
    const auto& unknown = m_hprof.GetUnknownRoots();

    printf("Objects with %s references:\n", incoming ? "incoming" : "outgoing");
    printf("Object Type                                                       Shallow Heap   Retained Heap\n");
    printf("================================================================  =============  =============\n");

    using ObjSP = std::shared_ptr<Object>;
    std::function<void(ObjSP, int, std::unordered_set<ObjSP> &)> showOneRef =
    [&](ObjSP obj, int depth, std::unordered_set<ObjSP> &memo) {
        if (obj->name.empty()) {
            /* not in heap, skip */
            return;
        }

        std::stringstream ss;
        ss << obj->name << " @ 0x" << std::hex << obj->id;
        if (locals.find(obj->id) != locals.end()) {
            ss << " [ROOT LOCAL]";
        } else if (globals.find(obj->id) != globals.end()) {
            ss << " [ROOT GLOBAL]";
        } else if (unknown.find(obj->id) != unknown.end()) {
            ss << " [ROOT UNKNOWN]";
        }
        /* Reserve 64 characters for the object type name, indent by 2 spaces each depth. */
        printf("%*s%-*s  %13llu  %13llu\n", depth * 2, "", 64 - depth * 2, ss.str().c_str(),
            (unsigned long long)obj->size, (unsigned long long)obj->retainedSize);

        auto ref = incoming ? obj->inRef : obj->outRef;
        if (!ref.empty() && ((depth >= maxDepth) || (memo.find(obj) != memo.end()))) {
            /* The maximum depth is exceeded, or cyclic reference is found, omitted, and indent by 2 spaces. */
            printf("%*s...\n", (depth + 1) * 2, "");
            return;
        }

        memo.emplace(obj);
        std::set<Hprof::ID> ordered_ref(ref.begin(), ref.end());
        for (auto r : ordered_ref) {
            auto m = memo;
            showOneRef(GetObject(r), depth + 1, m);
        }
    };

    for (auto obj : m_objects) {
        /* An empty objNames indicates that all names needs to be displayed. */
        if ((objNames.count(obj->name) != 0) || objNames.empty()) {
            std::unordered_set<ObjSP> memo;
            showOneRef(obj, 0, memo);
        }
    }

    printf("\n");
    auto t1 = std::chrono::steady_clock::now();
    auto showRefMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    LOG_INFO("[perf] ShowReference: {} ms", showRefMs);
    AddPhase("ShowReference", showRefMs);
}

void HeapAnalyzer::FilterPlaceholderObjects()
{
    std::unordered_set<Hprof::ID> removedIds;
    m_objects.erase(
        std::remove_if(m_objects.begin(), m_objects.end(),
            [&](const std::shared_ptr<Object>& obj) {
                if (obj->name.empty()) {
                    removedIds.insert(obj->id);
                    objects_cache.erase(obj->id);
                    return true;
                }
                return false;
            }),
        m_objects.end());

    for (auto& obj : m_objects) {
        for (auto it = obj->outRef.begin();it != obj->outRef.end();) {
            if (removedIds.count(*it)) {
                it = obj->outRef.erase(it);
            } else {
                ++it;
            }
        }
        for (auto it = obj->inRef.begin();it != obj->inRef.end();) {
            if (removedIds.count(*it)) {
                it = obj->inRef.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void HeapAnalyzer::AnalyzeObject()
{
    auto t0 = std::chrono::steady_clock::now();
    AnalyzeInstance();
    auto t1 = std::chrono::steady_clock::now();
    AnalyzeArray();
    auto t2 = std::chrono::steady_clock::now();

    if (!m_dumpReport) {
        /*
         * Traverses all objects. If the retained size of the current object has changed and the current object
         * is referenced by only one object, we need to apply the change to the retained size of that object.
         */
        for (auto obj : m_objects) {
            auto inc = obj->size;
            obj->retainedSize += inc;

            auto guard = obj;
            std::unordered_set<std::shared_ptr<Object>> memo;
            while (guard->inRef.size() == 1) {
                memo.emplace(guard);
                guard = GetObject(*guard->inRef.begin());
                if (memo.find(guard) != memo.end()) {
                    /* Cyclic reference found. */
                    break;
                }
            }

            while ((obj->inRef.size() == 1) && (obj != guard)) {
                obj = GetObject(*obj->inRef.begin());
                obj->retainedSize += inc;
            }
        }
    }
    auto t3 = std::chrono::steady_clock::now();

    auto t4 = std::chrono::steady_clock::now();
    if (!m_dumpReport) {
        /* Sorts objects by integer nameId for speed (local mapping, no persistent state). */
        std::unordered_map<std::string, uint32_t> nameIds;
        for (const auto& obj : m_objects) {
            nameIds.emplace(obj->name, 0);
        }
        std::vector<std::pair<std::string, uint32_t>> nameList(nameIds.begin(), nameIds.end());
        std::sort(nameList.begin(), nameList.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
        for (uint32_t i = 0; i < nameList.size(); ++i) {
            nameIds[nameList[i].first] = i;
        }

        std::vector<std::pair<uint32_t, std::shared_ptr<Object>>> sorted;
        sorted.reserve(m_objects.size());
        for (const auto& obj : m_objects) {
            sorted.emplace_back(nameIds[obj->name], obj);
        }
        std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
            return a.first == b.first ? a.second->id < b.second->id : a.first < b.first;
        });
        m_objects.clear();
        for (const auto& p : sorted) {
            m_objects.push_back(p.second);
        }
        t4 = std::chrono::steady_clock::now();
    }

    auto analyzeInstMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto analyzeArrMs = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    auto retainedSizeMs = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    auto sortMs = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();
    LOG_INFO("[perf]   AnalyzeInstance: {} ms", analyzeInstMs);
    LOG_INFO("[perf]   AnalyzeArray: {} ms", analyzeArrMs);
    LOG_INFO("[perf]   RetainedSize propagation: {} ms", retainedSizeMs);
    LOG_INFO("[perf]   Sort objects: {} ms", sortMs);
    AddPhase("AnalyzeInstance", analyzeInstMs);
    AddPhase("AnalyzeArray", analyzeArrMs);
    if (!m_dumpReport) {
        AddPhase("RetainedSize propagation", retainedSizeMs);
        AddPhase("Sort objects", sortMs);
    }
}

// std.core:func => std.core::func
static std::string ReplaceTypeName(const std::string &mangled)
{
    std::string demangled = mangled;
    size_t pos = demangled.find(':');
    while (pos != std::string::npos) {
        demangled.replace(pos, 1, "::");
        pos = demangled.find(':', pos + 2);
    }
    return demangled;
}

static const std::unordered_map<Hprof::BasicType, std::string> kPrimitiveArrayNames = {
    { Hprof::BasicType::BOOLEAN, "RawArray<Byte>[]" },
    { Hprof::BasicType::SHORT,   "RawArray<Harf>[]" },
    { Hprof::BasicType::INT,     "RawArray<Word>[]" },
    { Hprof::BasicType::LONG,    "RawArray<DWord>[]" }
};

void HeapAnalyzer::AnalyzeInstance()
{
    const auto& strings = m_hprof.GetStrings();
    const auto& classes = m_hprof.GetClasses();
    const auto& categories = m_hprof.GetObjectCategories();
    for (auto inst : m_hprof.GetInstances()) {
        auto obj = GetObject(inst.first);
        auto cls = classes.at(inst.second.cls);
        obj->id = inst.first;
        obj->name = ReplaceTypeName(GetStringSafe(strings, cls.name));
        obj->size = Align(cls.size);
        auto it = categories.find(inst.first);
        if (it != categories.end()) {
            obj->category = static_cast<uint8_t>(it->second);
        }
        for (auto id : inst.second.fields) {
            if (id == 0) {
                continue;
            }

            if (!obj->outRef.emplace(id).second) {
                continue;
            }
            auto ref = GetObject(id);
            ref->inRef.emplace(obj->id);
        }
    }
}

void HeapAnalyzer::AnalyzeArray()
{
    const auto& strings = m_hprof.GetStrings();
    const auto& classes = m_hprof.GetClasses();
    const auto& componentNums = m_hprof.GetComponentNums();
    const auto& categories = m_hprof.GetObjectCategories();
    for (auto arr : m_hprof.GetArrays()) {
        auto obj = GetObject(arr.first);
        obj->id = arr.first;
        obj->size = arr.second.GetFixedSize();
        auto it = categories.find(arr.first);
        if (it != categories.end()) {
            obj->category = static_cast<uint8_t>(it->second);
        }

        if (arr.second.type != Hprof::BasicType::OBJECT) {
            auto nameIt = kPrimitiveArrayNames.find(arr.second.type);
            if (nameIt != kPrimitiveArrayNames.end()) {
                obj->name = nameIt->second;
            }
            obj->size += Align(uint64_t(arr.second.type == Hprof::BasicType::BOOLEAN ? 1 :
                arr.second.type == Hprof::BasicType::SHORT ? 2 :
                arr.second.type == Hprof::BasicType::INT ? 4 :
                arr.second.type == Hprof::BasicType::LONG ? 8 : 0) * arr.second.num);
            continue;
        }

        auto cls = classes.at(arr.second.cls);
        obj->name = ReplaceTypeName(GetStringSafe(strings, cls.name));
        obj->size += cls.size != 0 ?
            Align(uint64_t(cls.size) * (componentNums.find(obj->id) != componentNums.end() ?
                componentNums.at(obj->id)
                : arr.second.num))
            : sizeof(Hprof::ID) * arr.second.num;
        for (auto element : arr.second.elements) {
            if (element == 0) {
                continue;
            }

            obj->outRef.emplace(element);
            auto ref = GetObject(element);
            ref->inRef.emplace(obj->id);
        }
    }
}

void HeapAnalyzer::AnalyzeThread()
{
    const auto& strings = m_hprof.GetStrings();
    auto frames = m_hprof.GetFrames();
    std::map<Hprof::u4, std::vector<Frame>> stackTraces;
    for (auto stackTrace : m_hprof.GetStackTraces()) {
        for (auto frame : stackTrace.second.frames) {
            stackTraces[stackTrace.first].push_back(
                { GetStringSafe(strings, frames[frame].name),
                  GetStringSafe(strings, frames[frame].fileName),
                  int(frames[frame].line)
                }
            );
        }
    }

    std::map<Hprof::u4, Thread> threads;
    for (auto thread : m_hprof.GetThreads()) {
        threads[thread.second.idx] =
            { GetStringSafe(strings, thread.second.name), stackTraces[thread.second.stackTraceIdx] };
    }

    for (auto local : m_hprof.GetLocalsRoots()) {
        auto thread = local.second.thread;
        if (threads.find(thread) == threads.end()) {
            continue;
        }

        auto frame = local.second.frame;
        if (frame < threads[thread].frames.size()) {
            threads[thread].frames[frame].locals.push_back(GetObject(local.first));
        }
    }

    for (auto thread : threads) {
        m_threads.push_back(thread.second);
    }

    /* Sorts threads in lexicographic order by thread name. */
    std::sort(m_threads.begin(), m_threads.end(), [](const Thread &a, const Thread &b) { return a.name < b.name; });
}

std::shared_ptr<HeapAnalyzer::Object> HeapAnalyzer::GetObject(Hprof::ID id)
{
    auto obj = objects_cache[id];
    if (obj) {
        return obj;
    }

    objects_cache[id] = obj = std::make_shared<Object>(id);
    m_objects.push_back(obj);
    return obj;
}

static uint64_t GetRawHeapNameIndex(
    RawHeapSnapshot& data, std::string& name, std::unordered_map<uint64_t, uint32_t>& nameIndexs)
{
    uint32_t nameIndex = 0;
    auto nameid = std::hash<std::string>()(name);
    auto pos = nameIndexs.find(nameid);
    if (pos == nameIndexs.end()) {
        nameIndex = data.strings.size();
        data.strings.emplace_back(name);
        nameIndexs.emplace(nameid, nameIndex);
    } else {
        nameIndex = nameIndexs.at(nameid);
    }
    return nameIndex;
}

static void GetRawHeapNodeType(
    RawHeapSnapshot::Node& node,
    const std::unordered_map<Hprof::ID, Hprof::Local>& locals,
    const std::unordered_set<Hprof::ID>& globals,
    const std::unordered_set<Hprof::ID>& unknown,
    const std::unordered_map<Hprof::ID, Hprof::Array>& arrays)
{
    if (arrays.find(node.id) != arrays.end()) {
        node.type = RawHeapSnapshot::NodeType::ARRAY;
        auto arr = arrays.at(node.id);
        node.arrayLen = arr.num;
    } else {
        node.type = RawHeapSnapshot::NodeType::OBJECT;
    }

    if (locals.find(node.id) != locals.end()) {
        node.rootType = RawHeapSnapshot::RootType::LOCAL;
        return;
    }

    if (globals.find(node.id) != globals.end()) {
        node.rootType = RawHeapSnapshot::RootType::GLOBAL;
        return;
    }

    if (unknown.find(node.id) != unknown.end()) {
        node.rootType = RawHeapSnapshot::RootType::UNKNOWN;
    }
}

RawHeapSnapshot HeapAnalyzer::GetRawHeapSnapshot()
{
    // hashid of obj name, name index in strings
    std::unordered_map<uint64_t, uint32_t> nameIndexs;
    // obj id, node index in nodes
    std::unordered_map<uint64_t, uint32_t> nodeIndexs;

    const auto& locals = m_hprof.GetLocalsRoots();
    const auto& globals = m_hprof.GetGlobalsRoots();
    const auto& unknown = m_hprof.GetUnknownRoots();
    const auto& arrays = m_hprof.GetArrays();

    RawHeapSnapshot data = {};
    for (auto obj : m_objects) {
        RawHeapSnapshot::Node node = {};
        node.id = obj->id;
        GetRawHeapNodeType(node, locals, globals, unknown, arrays);
        node.isPinned = (obj->category == static_cast<uint8_t>(ObjectCategory::PINNED_OBJECT));
        node.selfSize = obj->size;
        node.edgeCount = obj->outRef.size();
        node.nameIndex = GetRawHeapNameIndex(data, obj->name, nameIndexs);
        nodeIndexs.emplace(obj->id, data.nodes.size());
        node.nodeIndex = data.nodes.size();
        data.nodes.emplace_back(node);
    }

    for (auto obj : m_objects) {
        for (auto refId : obj->outRef) {
            auto refNode = GetObject(refId);
            if (nodeIndexs.find(refNode->id) == nodeIndexs.end()) {
                continue;
            }
            RawHeapSnapshot::Edge edge = {};
            edge.toNode = nodeIndexs.at(refNode->id);
            data.edges.emplace_back(edge);
        }
    }

    data.nodeCount = data.nodes.size();
    data.edgeCount = data.edges.size();
    data.startTime = m_hprof.GetFileTime();
    data.fileSize = m_fileSize;

    for (auto th : m_threads) {
        RawHeapSnapshot::Thread rawth;
        rawth.name = th.name;
        for (auto fr : th.frames) {
            RawHeapSnapshot::Frame rawfr;
            rawfr.funcName = GetRawHeapNameIndex(data, fr.name, nameIndexs);
            rawfr.fileName = GetRawHeapNameIndex(data, fr.fileName, nameIndexs);
            rawfr.line = fr.line;
            for (auto obj : fr.locals) {
                if (nodeIndexs.find(obj->id) == nodeIndexs.end()) {
                    continue;
                }
                auto nodeIndex = nodeIndexs.at(obj->id);
                rawfr.locals.emplace_back(nodeIndex);
            }
            rawth.frames.emplace_back(rawfr);
        }
        data.threads.emplace_back(rawth);
    }

    return data;
}

void RawHeapSnapshot::PrintSummary() {
    size_t totalSize = 0;
    int objectCount = 0, arrayCount = 0, stringCount = 0;

    for (const auto& node : nodes) {
        totalSize += node.selfSize;
        // 节点类型名称
        // 类型值    类型名                  说明
        // 0        hidden                  隐藏节点（内部使用）
        // 1        array                   数组
        // 2        string                  字符串
        // 3        object                  普通对象
        // 4        code                    代码对象
        // 5        closure                 闭包
        // 6        regexp                  正则表达式
        // 7        number                  数字
        // 8        native                  原生对象
        // 9        synthetic               合成对象
        // 10       concatenated string     拼接字符串
        // 11       sliced string           切片字符串
        // 12       symbol                  Symbol
        // 13       bigint                  BigInt
        switch (node.type) {
            case NodeType::ARRAY: arrayCount++; break;
            case NodeType::STRING: stringCount++; break;
            case NodeType::OBJECT: objectCount++; break;
        }
        // 边类型：edge.type
        // 类型值        类型名             说明
        // 0            context            上下文引用
        // 1            element            数组元素
        // 2            property           对象属性
        // 3            internal           内部引用
        // 4            hidden             隐藏引用
        // 5            shortcut           快捷引用
        // 6            weak               弱引用
    }

    std::stringstream ss;
    ss << "================================================================================\n";
    ss << "RawHeapSnapshot data, filePath: " << filePath << ", id: " << hashid << ", hashid: " << hashid << "\n";
    ss << "RawHeapSnapshot data, nodeCount: " << nodeCount << ", edgeCount: " << edgeCount
            << ", startTime: " << startTime << ", fileSize: " << fileSize << "\n";
    ss << "\n=== Heap Snapshot Summary ===" << "\n";
    ss << "Total nodes: " << nodes.size() << "\n";
    ss << "Total size: " << totalSize << " bytes" << "\n";
    ss << "  Arrays: " << arrayCount << "\n";
    ss << "  Objects: " << objectCount << "\n";
    ss << "  Strings: " << stringCount << "\n";
    for (const auto& node : nodes) {
        ss << "RawHeapSnapshot node"
            << ", id: " << node.id << ", type: " <<node.TypeString() << ", rootType: " << node.RootString()
            << ", self_size: " << node.selfSize <<", arraylen: " << node.arrayLen << ", edge_count: " << node.edgeCount
            << ", name_index: " << node.nameIndex <<", name: " << strings[node.nameIndex] << "\n";
    }
    for (auto th : threads) {
        for (auto fr : th.frames) {
            for (auto nodeid : fr.locals) {
                auto node = nodes[nodeid];
                ss << "RawHeapSnapshot threads obj" << ", id: " << node.id
                    << ", name_index: " << node.nameIndex << ", name: " << strings[node.nameIndex] << "\n";
            }
        }
    }
    ss << "================================================================================\n";
    printf("%s\n", ss.str().c_str());
}

#include "Analyzer/Types.h"
#include "Analyzer/HttpContext.h"
#include "Analyzer/HttpServer.h"
#include "Analyzer/DatabaseCache.h"
#include "Cjprof.h"
#include <thread>
#include <chrono>

static uint64_t ComputeInstanceSize(const Hprof::Instance& inst,
                                    const std::unordered_map<Hprof::ID, Hprof::Class>& classes)
{
    auto it = classes.find(inst.cls);
    return (it != classes.end()) ? ((it->second.size + 7) & ~7ULL) : 0;
}

static uint64_t ComputeArraySize(Hprof::ID id, const Hprof::Array& arr,
                                 const std::unordered_map<Hprof::ID, Hprof::Class>& classes,
                                 const std::unordered_map<Hprof::ID, Hprof::u4>& componentNums)
{
    uint64_t size = arr.GetFixedSize();
    if (arr.type == Hprof::BasicType::OBJECT) {
        auto it = classes.find(arr.cls);
        if (it != classes.end() && it->second.size != 0) {
            uint64_t num = arr.num;
            auto compIt = componentNums.find(id);
            if (compIt != componentNums.end()) {
                num = compIt->second;
            }
            size += ((uint64_t(it->second.size) * num) + 7) & ~7ULL;
        } else {
            size += sizeof(Hprof::ID) * arr.num;
        }
    } else {
        int elemSize = 0;
        switch (arr.type) {
            case Hprof::BasicType::BOOLEAN: elemSize = 1; break;
            case Hprof::BasicType::SHORT:   elemSize = 2; break;
            case Hprof::BasicType::INT:     elemSize = 4; break;
            case Hprof::BasicType::LONG:    elemSize = 8; break;
            default: break;
        }
        if (elemSize > 0) {
            size += ((uint64_t(elemSize) * arr.num) + 7) & ~7ULL;
        }
    }
    return size;
}

static std::vector<cjprof::DominanceNode> BuildDominanceNodesFromHprof(const Hprof& hprof, const std::unordered_map<uint64_t, uint64_t>& objectSizes)
{
    using namespace cjprof;
    const auto& instances = hprof.GetInstances();
    const auto& arrays = hprof.GetArrays();
    const auto& classes = hprof.GetClasses();
    const auto& componentNums = hprof.GetComponentNums();
    const auto& locals = hprof.GetLocalsRoots();
    const auto& globals = hprof.GetGlobalsRoots();
    const auto& unknown = hprof.GetUnknownRoots();
    const auto& categories = hprof.GetObjectCategories();

    size_t n = instances.size() + arrays.size();
    if (n == 0) {
        return {};
    }

    // Step 1: Build id -> index mapping and size vector
    std::unordered_map<uint64_t, size_t> idToIndex;
    idToIndex.reserve(n * 2);
    std::vector<uint64_t> sizes;
    sizes.reserve(n);
    for (const auto& inst : instances) {
        idToIndex[inst.first] = sizes.size();
        sizes.push_back(objectSizes.at(inst.first));
    }

    for (const auto& arr : arrays) {
        idToIndex[arr.first] = sizes.size();
        sizes.push_back(objectSizes.at(arr.first));
    }

    // Step 2: Build succs/preds from raw references
    std::vector<std::vector<size_t>> succs(n);
    std::vector<std::vector<size_t>> preds(n);

    size_t totalRefs = 0;
    size_t validRefs = 0;
    for (const auto& inst : instances) {
        auto itFrom = idToIndex.find(inst.first);
        if (itFrom == idToIndex.end()) continue;
        size_t from = itFrom->second;
        for (auto refId : inst.second.fields) {
            totalRefs++;
            if (refId == 0) continue;
            auto itTo = idToIndex.find(refId);
            if (itTo != idToIndex.end()) {
                size_t to = itTo->second;
                succs[from].push_back(to);
                preds[to].push_back(from);
                validRefs++;
            }
        }
    }

    for (const auto& arr : arrays) {
        if (arr.second.type != Hprof::BasicType::OBJECT) continue;
        auto itFrom = idToIndex.find(arr.first);
        if (itFrom == idToIndex.end()) continue;
        size_t from = itFrom->second;
        for (auto element : arr.second.elements) {
            totalRefs++;
            if (element == 0) continue;
            auto itTo = idToIndex.find(element);
            if (itTo != idToIndex.end()) {
                size_t to = itTo->second;
                succs[from].push_back(to);
                preds[to].push_back(from);
                validRefs++;
            }
        }
    }

    LOG_INFO("Reference graph: {} total refs, {} valid refs (target in idToIndex)", totalRefs, validRefs);

    // Count nodes with preds
    size_t nodesWithPreds = 0;
    for (size_t i = 0; i < n; ++i) {
        if (!preds[i].empty()) nodesWithPreds++;
    }
    LOG_INFO("Reference graph: {} nodes have preds (are referenced), {} nodes have no preds (potential roots)",
             nodesWithPreds, n - nodesWithPreds);

    // Step 3: Identify GC Roots
    std::vector<size_t> gcRoots;
    for (size_t i = 0; i < n; ++i) {
        if (preds[i].empty()) {
            gcRoots.push_back(i);
        }
    }
    // Also add explicit roots from hprof
    for (const auto& local : locals) {
        auto it = idToIndex.find(local.first);
        if (it != idToIndex.end() && std::find(gcRoots.begin(), gcRoots.end(), it->second) == gcRoots.end()) {
            gcRoots.push_back(it->second);
        }
    }
    for (auto global : globals) {
        auto it = idToIndex.find(global);
        if (it != idToIndex.end() && std::find(gcRoots.begin(), gcRoots.end(), it->second) == gcRoots.end()) {
            gcRoots.push_back(it->second);
        }
    }
    for (auto unk : unknown) {
        auto it = idToIndex.find(unk);
        if (it != idToIndex.end() && std::find(gcRoots.begin(), gcRoots.end(), it->second) == gcRoots.end()) {
            gcRoots.push_back(it->second);
        }
    }
    for (const auto& inst : instances) {
        auto catIt = categories.find(inst.first);
        if (catIt != categories.end() && catIt->second == ObjectCategory::PINNED_OBJECT) {
            auto it = idToIndex.find(inst.first);
            if (it != idToIndex.end() && std::find(gcRoots.begin(), gcRoots.end(), it->second) == gcRoots.end()) {
                gcRoots.push_back(it->second);
            }
        }
    }

    if (gcRoots.empty()) {
        return {};
    }

    // Step 4: Compute dominance tree
    auto result = Cjprof::ComputeDominanceTree(n, succs, preds, gcRoots);
    const auto& dom = result.dom;
    const auto& domTree = result.domTree;
    size_t entry = 0;

    // Debug: check dominance tree structure
    size_t rootCount = 0;
    for (size_t v = 1; v <= n; ++v) {
        if (dom[v] == entry) rootCount++;
    }
    LOG_INFO("Dominance tree: {} nodes, {} roots (dom[v]==entry), {} children of entry in domTree",
             n, rootCount, domTree[entry].size());

    // Step 5: Compute retainedSize recursively
    std::vector<uint64_t> retainedSizes(n, 0);
    std::function<uint64_t(size_t)> computeRetainedSize = [&](size_t node) -> uint64_t {
        size_t originalIdx = node - 1;
        uint64_t size = sizes[originalIdx];
        for (size_t child : domTree[node]) {
            size += computeRetainedSize(child);
        }
        retainedSizes[originalIdx] = size;
        return size;
    };
    for (size_t v = 1; v <= n; ++v) {
        if (dom[v] == entry) {
            computeRetainedSize(v);
        }
    }

    // unreachable nodes: retained = shallow
    for (size_t i = 0; i < n; ++i) {
        if (dom[i + 1] == result.noEntry) {
            retainedSizes[i] = sizes[i];
        }
    }

    // Step 6: Compute depth via BFS from roots
    std::vector<uint32_t> depth(n, UINT32_MAX);
    std::queue<size_t> bfs;
    for (size_t i = 0; i < n; ++i) {
        if (dom[i + 1] == entry) {
            depth[i] = 0;
            bfs.push(i);
        }
    }
    while (!bfs.empty()) {
        size_t idx = bfs.front();
        bfs.pop();
        for (size_t childOrdinal : domTree[idx + 1]) {
            size_t childIdx = childOrdinal - 1;
            if (depth[childIdx] == UINT32_MAX || depth[childIdx] > depth[idx] + 1) {
                depth[childIdx] = depth[idx] + 1;
                bfs.push(childIdx);
            }
        }
    }

    // Step 7: Build DominanceNode array
    // Need a vector of IDs in index order
    std::vector<uint64_t> ids(n);
    for (const auto& inst : instances) {
        ids[idToIndex[inst.first]] = inst.first;
    }
    for (const auto& arr : arrays) {
        ids[idToIndex[arr.first]] = arr.first;
    }

    std::vector<DominanceNode> nodes;
    nodes.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        DominanceNode dn;
        dn.object_id = ids[i];
        dn.shallow_size = sizes[i];
        dn.retained_size = retainedSizes[i];
        dn.depth = depth[i] == UINT32_MAX ? 0 : depth[i];
        size_t parentOrdinal = dom[i + 1];
        if (parentOrdinal == result.noEntry) {
            dn.parent_id = 0;
        } else {
            dn.parent_id = (parentOrdinal > 0 && parentOrdinal <= n) ? ids[parentOrdinal - 1] : 0;
        }
        dn.instance_count = 1;
        dn.is_class_clustered = false;
        nodes.push_back(dn);
    }

    return nodes;
}

static std::vector<cjprof::DominanceNode> BuildDominanceNodes(const RawHeapSnapshot& rhs)
{
    using namespace cjprof;
    size_t n = rhs.nodes.size();
    if (n == 0) {
        return {};
    }

    // Build children (succs) and parents (preds) from edges
    std::vector<std::vector<size_t>> succs(n);
    std::vector<std::vector<size_t>> preds(n);
    size_t edgeIndex = 0;
    for (size_t i = 0; i < n; ++i) {
        for (uint32_t j = 0; j < rhs.nodes[i].edgeCount && edgeIndex < rhs.edges.size(); ++j) {
            size_t to = rhs.edges[edgeIndex++].toNode;
            if (to < n) {
                succs[i].push_back(to);
                preds[to].push_back(i);
            }
        }
    }

    // Identify GC Roots
    std::vector<size_t> gcRoots;
    for (size_t i = 0; i < n; ++i) {
        if (preds[i].empty() || rhs.nodes[i].IsRoot() || rhs.nodes[i].isPinned) {
            gcRoots.push_back(i);
        }
    }

    if (gcRoots.empty()) {
        return {};
    }

    // Reuse the project's Cooper algorithm via ComputeDominanceTree
    auto result = Cjprof::ComputeDominanceTree(n, succs, preds, gcRoots);
    const auto& dom = result.dom;
    const auto& domTree = result.domTree;
    size_t entry = 0;

    // Compute retainedSize recursively
    std::vector<uint64_t> retainedSizes(n, 0);
    std::function<uint64_t(size_t)> computeRetainedSize = [&](size_t node) -> uint64_t {
        size_t originalIdx = node - 1;
        uint64_t size = rhs.nodes[originalIdx].selfSize;
        for (size_t child : domTree[node]) {
            size += computeRetainedSize(child);
        }
        retainedSizes[originalIdx] = size;
        return size;
    };
    for (size_t v = 1; v <= n; ++v) {
        if (dom[v] == entry) {
            computeRetainedSize(v);
        }
    }

    // unreachable nodes: retained = shallow
    for (size_t i = 0; i < n; ++i) {
        if (dom[i + 1] == result.noEntry) {
            retainedSizes[i] = rhs.nodes[i].selfSize;
        }
    }

    // Compute depth via BFS from roots
    std::vector<uint32_t> depth(n, UINT32_MAX);
    std::queue<size_t> bfs;
    for (size_t i = 0; i < n; ++i) {
        if (dom[i + 1] == entry) {
            depth[i] = 0;
            bfs.push(i);
        }
    }
    while (!bfs.empty()) {
        size_t idx = bfs.front();
        bfs.pop();
        for (size_t childOrdinal : domTree[idx + 1]) {
            size_t childIdx = childOrdinal - 1;
            if (depth[childIdx] == UINT32_MAX || depth[childIdx] > depth[idx] + 1) {
                depth[childIdx] = depth[idx] + 1;
                bfs.push(childIdx);
            }
        }
    }

    // Build DominanceNode array
    std::vector<DominanceNode> nodes;
    nodes.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        DominanceNode dn;
        dn.object_id = rhs.nodes[i].id;
        dn.shallow_size = rhs.nodes[i].selfSize;
        dn.retained_size = retainedSizes[i];
        dn.depth = depth[i] == UINT32_MAX ? 0 : depth[i];
        size_t parentOrdinal = dom[i + 1];
        if (parentOrdinal == result.noEntry) {
            dn.parent_id = 0;
        } else {
            dn.parent_id = (parentOrdinal > 0 && parentOrdinal <= n) ? rhs.nodes[parentOrdinal - 1].id : 0;
        }
        dn.instance_count = 1;
        dn.is_class_clustered = false;
        nodes.push_back(dn);
    }
    return nodes;
}

std::chrono::steady_clock::time_point g_startTime;

void HeapAnalyzer::SetProgramStartTime()
{
    g_startTime = std::chrono::steady_clock::now();
    g_phaseBreakdown.clear();
}

bool HeapAnalyzer::StartReportServer(int port)
{
    using namespace cjprof;
    auto t_total = std::chrono::steady_clock::now();
    initLogger();

    std::vector<HeapObject> heapObjects;
    std::vector<ClassInfo> classInfos;
    std::vector<GcRoot> gcRoots;
    std::vector<DominanceNode> dominanceNodes;
    SnapshotInfo snapshotInfo;
    std::unordered_map<uint64_t, std::string> stringTable;

    // Check for cached database (Section 9.1: Persistent cache)
    bool cacheLoaded = false;
    auto t0 = std::chrono::steady_clock::now();
    auto initMs = std::chrono::duration_cast<std::chrono::milliseconds>(t0 - t_total).count();
    LOG_INFO("[perf] StartReportServer init: {} ms", initMs);
    AddPhase("StartReportServer init", initMs);

    if (DatabaseCache::isCacheValid(m_filePath)) {
        auto t1 = std::chrono::steady_clock::now();
        LOG_DEBUG("Cache found: {}.cjprof.db, loading...", m_filePath);
        if (DatabaseCache::load(m_filePath, snapshotInfo, classInfos, heapObjects, gcRoots, dominanceNodes, stringTable)) {
            cacheLoaded = true;
            auto t2 = std::chrono::steady_clock::now();
            LOG_DEBUG("Cache loaded successfully (objects={}, roots={}, dominance_nodes={})",
                     heapObjects.size(), gcRoots.size(), dominanceNodes.size());
            auto isValidMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            auto loadMs = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            LOG_INFO("[perf] DatabaseCache::isCacheValid: {} ms", isValidMs);
            LOG_INFO("[perf] DatabaseCache::load: {} ms", loadMs);
            AddPhase("DatabaseCache::isCacheValid", isValidMs);
            AddPhase("DatabaseCache::load", loadMs);
        } else {
            LOG_WARN("Cache load failed, falling back to full parse");
        }
    }

    if (!cacheLoaded) {
        LOG_DEBUG("No cache found, parsing heap file...");

        const auto& hprofClasses = m_hprof.GetClasses();
        const auto& hprofStrings = m_hprof.GetStrings();
        const auto& locals = m_hprof.GetLocalsRoots();
        const auto& globals = m_hprof.GetGlobalsRoots();
        const auto& unknown = m_hprof.GetUnknownRoots();
        const auto& categories = m_hprof.GetObjectCategories();
        const auto& componentNums = m_hprof.GetComponentNums();
        const auto& instances = m_hprof.GetInstances();
        const auto& arrays = m_hprof.GetArrays();

        if (instances.empty() && arrays.empty()) {
            LOG_DEBUG("No parsed data available, skip building HTTP context");
            if (!g_phaseBreakdown.empty() && g_phaseBreakdown.back().first == "StartReportServer init") {
                g_phaseBreakdown.pop_back();
            }
            return false;
        }

        // Build HeapObject list directly from m_hprof (skip m_objects intermediary)
        auto t1 = std::chrono::steady_clock::now();
        std::unordered_map<uint64_t, uint64_t> objectSizes;
        objectSizes.reserve(instances.size() + arrays.size());
        heapObjects.reserve(instances.size() + arrays.size());
        uint64_t totalUsedSize = 0;
        for (const auto& inst : instances) {
            HeapObject ho;
            ho.object_id = inst.first;
            auto it = hprofClasses.find(inst.second.cls);
            if (it != hprofClasses.end()) {
                ho.class_id = inst.second.cls;
                auto nameIt = hprofStrings.find(it->second.name);
                if (nameIt != hprofStrings.end()) {
                    ho.name = ReplaceTypeName(nameIt->second);
                }
            }
            for (auto id : inst.second.fields) {
                if (id != 0) ho.refs.push_back(id);
            }
            auto catIt = categories.find(inst.first);
            if (catIt != categories.end()) {
                ho.category = static_cast<ObjectCategory>(catIt->second);
            }
            uint64_t size = ComputeInstanceSize(inst.second, hprofClasses);
            ho.size = size;
            objectSizes[inst.first] = size;
            totalUsedSize += size;
            heapObjects.push_back(ho);
        }
        for (const auto& arr : arrays) {
            HeapObject ho;
            ho.object_id = arr.first;
            if (arr.second.type == Hprof::BasicType::OBJECT) {
                auto it = hprofClasses.find(arr.second.cls);
                if (it != hprofClasses.end()) {
                    ho.class_id = arr.second.cls;
                    auto nameIt = hprofStrings.find(it->second.name);
                    if (nameIt != hprofStrings.end()) {
                        ho.name = ReplaceTypeName(nameIt->second);
                    }
                }
                for (auto element : arr.second.elements) {
                    if (element != 0) ho.refs.push_back(element);
                }
            } else {
                // Primitive array: set name
                auto nameIt = kPrimitiveArrayNames.find(arr.second.type);
                if (nameIt != kPrimitiveArrayNames.end()) {
                    ho.name = nameIt->second;
                }
            }
            auto catIt = categories.find(arr.first);
            if (catIt != categories.end()) {
                ho.category = static_cast<ObjectCategory>(catIt->second);
            }
            uint64_t size = ComputeArraySize(arr.first, arr.second, hprofClasses, componentNums);
            ho.size = size;
            objectSizes[arr.first] = size;
            totalUsedSize += size;
            heapObjects.push_back(ho);
        }
        auto t2 = std::chrono::steady_clock::now();

        // Build ClassInfo list
        for (auto& cls : hprofClasses) {
            ClassInfo ci;
            ci.class_id = cls.first;
            ci.name_id = cls.second.name;
            auto it = hprofStrings.find(cls.second.name);
            if (it != hprofStrings.end()) {
                ci.class_name = it->second;
            }
            ci.size = cls.second.size;
            classInfos.push_back(ci);
        }
        auto t3 = std::chrono::steady_clock::now();

        // Build GcRoot list
        for (auto& local : locals) {
            GcRoot root;
            root.object_id = local.first;
            root.type = RootType::LOCAL;
            root.thread_idx = local.second.thread;
            root.frame_idx = local.second.frame;
            gcRoots.push_back(root);
        }
        for (auto& global : globals) {
            GcRoot root;
            root.object_id = global;
            root.type = RootType::GLOBAL;
            gcRoots.push_back(root);
        }
        for (auto& unk : unknown) {
            GcRoot root;
            root.object_id = unk;
            root.type = RootType::UNKNOWN;
            gcRoots.push_back(root);
        }
        auto t4 = std::chrono::steady_clock::now();

        // Build SnapshotInfo
        snapshotInfo.object_count = instances.size() + arrays.size();
        snapshotInfo.gc_root_count = gcRoots.size();
        snapshotInfo.heap_total_size = 512ULL * 1024 * 1024;
        auto t5 = std::chrono::steady_clock::now();

        // Build dominance tree directly from m_hprof (skip RawHeapSnapshot intermediary)
        dominanceNodes = BuildDominanceNodesFromHprof(m_hprof, objectSizes);
        snapshotInfo.used_size = totalUsedSize;
        auto t6 = std::chrono::steady_clock::now();

        // Build string table
        for (auto& s : hprofStrings) {
            stringTable[s.first] = s.second;
        }
        auto t7 = std::chrono::steady_clock::now();

        auto buildHeapMs = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        auto buildClassMs = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
        auto buildGcRootMs = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();
        auto buildSnapshotMs = std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4).count();
        auto buildDomMs = std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t5).count();
        auto buildStringMs = std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6).count();
        LOG_INFO("[perf] Build HeapObjects: {} ms", buildHeapMs);
        LOG_INFO("[perf] Build ClassInfos: {} ms", buildClassMs);
        LOG_INFO("[perf] Build GcRoots: {} ms", buildGcRootMs);
        LOG_INFO("[perf] Build SnapshotInfo: {} ms", buildSnapshotMs);
        LOG_INFO("[perf] BuildDominanceNodes: {} ms", buildDomMs);
        LOG_INFO("[perf] Build stringTable: {} ms", buildStringMs);
        AddPhase("Build HeapObjects", buildHeapMs);
        AddPhase("Build ClassInfos", buildClassMs);
        AddPhase("Build GcRoots", buildGcRootMs);
        AddPhase("Build SnapshotInfo", buildSnapshotMs);
        AddPhase("BuildDominanceNodes", buildDomMs);
        AddPhase("Build stringTable", buildStringMs);
    }

    auto t_after_build = std::chrono::steady_clock::now();

    // Move data to heap to extend lifetime for async background save
    auto sharedHeapObjects = std::make_shared<std::vector<HeapObject>>(std::move(heapObjects));
    auto sharedClassInfos = std::make_shared<std::vector<ClassInfo>>(std::move(classInfos));
    auto sharedGcRoots = std::make_shared<std::vector<GcRoot>>(std::move(gcRoots));
    auto sharedDominanceNodes = std::make_shared<std::vector<DominanceNode>>(std::move(dominanceNodes));
    auto sharedSnapshotInfo = std::make_shared<SnapshotInfo>(std::move(snapshotInfo));

    // Create HttpContext
    auto context = std::make_shared<HttpContext>();
    context->classes = sharedClassInfos.get();
    context->objects = sharedHeapObjects.get();
    context->gcRoots = sharedGcRoots.get();
    context->dominanceNodes = sharedDominanceNodes.get();
    context->snapshotInfo = sharedSnapshotInfo.get();
    context->stringTable = &stringTable;

    // Build indexes for O(1)/O(children) lookup in handlers (one-time O(N) cost)
    auto t_index_start = std::chrono::steady_clock::now();
    for (const auto& node : *context->dominanceNodes) {
        context->objectIdToRetainedSize[node.object_id] = node.retained_size;
        context->objectIdToDominanceNode[node.object_id] = &node;
        if (node.parent_id != 0) {
            context->childrenByParentId[node.parent_id].push_back(&node);
        }
    }
    // Build object_id -> class_name index (reuse buildObjectIdToClassMap logic inline)
    // Also build className -> DominanceNode* index for by-type handlers
    {
        std::unordered_map<uint64_t, std::string> classIdToNameMap;
        if (context->classes) {
            for (const auto& cls : *context->classes) {
                if (!cls.class_name.empty()) {
                    classIdToNameMap[cls.class_id] = cls.class_name;
                }
            }
        }
        if (context->objects) {
            for (const auto& obj : *context->objects) {
                std::string className;
                if (!obj.name.empty()) {
                    className = obj.name;
                } else if (obj.class_id == 0) {
                    switch (obj.category) {
                        case ObjectCategory::PRIMITIVE_ARRAY: className = "PRIMITIVE_ARRAY"; break;
                        case ObjectCategory::OBJECT_ARRAY: className = "OBJECT_ARRAY"; break;
                        case ObjectCategory::STRUCT_ARRAY: className = "STRUCT_ARRAY"; break;
                        case ObjectCategory::PINNED_OBJECT: className = "PINNED_OBJECT"; break;
                        case ObjectCategory::LARGE_OBJECT: className = "LARGE_OBJECT"; break;
                        case ObjectCategory::UNMOVABLE_OBJECT: className = "UNMOVABLE_OBJECT"; break;
                        default: className = "unknown"; break;
                    }
                } else {
                    auto it = classIdToNameMap.find(obj.class_id);
                    className = (it != classIdToNameMap.end()) ? it->second : "unknown";
                }
                context->objectIdToClassName[obj.object_id] = className;
                // Build className -> DominanceNode* index (only for objects in dominance tree)
                if (className != "unknown") {
                    auto dnIt = context->objectIdToDominanceNode.find(obj.object_id);
                    if (dnIt != context->objectIdToDominanceNode.end()) {
                        context->classNameToDominanceNodes[className].push_back(dnIt->second);
                    }
                }
            }
        }
    }
    auto t_index_end = std::chrono::steady_clock::now();
    auto indexMs = std::chrono::duration_cast<std::chrono::milliseconds>(t_index_end - t_index_start).count();
    LOG_INFO("[perf] Build indexes: {} ms", indexMs);
    AddPhase("Build indexes", indexMs);

    // Find available port
    int actualPort = port;
    for (int attempt = 0; attempt < 100; attempt++) {
        if (!HttpServer::isPortInUse(actualPort)) {
            break;
        }
        actualPort++;
    }

    // Start server
    HttpServer server(actualPort);
    server.setContext(context);
    server.start();

    // Background cache save (does not block ready time)
    if (!cacheLoaded) {
        std::thread([filePath = m_filePath,
                     sharedSnapshotInfo,
                     sharedClassInfos,
                     sharedHeapObjects,
                     sharedGcRoots,
                     sharedDominanceNodes]() {
            LOG_INFO("[perf] Background cache save started...");
            auto t0 = std::chrono::steady_clock::now();
            bool ok = DatabaseCache::save(filePath,
                *sharedSnapshotInfo,
                *sharedClassInfos,
                *sharedHeapObjects,
                *sharedGcRoots,
                *sharedDominanceNodes);
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t0).count();
            if (ok) {
                LOG_INFO("[perf] Background cache save done: {} ms", elapsed);
            } else {
                LOG_WARN("Background cache save failed");
            }
        }).detach();
    }

    auto t_ready = std::chrono::steady_clock::now();
    auto finalizeMs = std::chrono::duration_cast<std::chrono::milliseconds>(t_ready - t_after_build).count();
    LOG_INFO("[perf] StartReportServer finalize: {} ms", finalizeMs);
    AddPhase("StartReportServer finalize", finalizeMs);

    auto elapsed = t_ready - g_startTime;
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    LOG_INFO("[perf] Total elapsed from program start to ready: {} ms", elapsedMs);
    LOG_INFO("cjprof ready! (elapsed {} ms)", elapsedMs);
    LOG_INFO("Access URL: http://localhost:{}", actualPort);
    LOG_INFO("Press Ctrl+C to stop");
    PrintPhaseBreakdown(elapsedMs);

    // Keep running until Ctrl+C
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return true;
}
