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
#include "Analyzer/HeapAnalyzer.h"

bool HeapAnalyzer::SetData(const std::string &file)
{
    std::ifstream ifs(file, std::ifstream::binary);
    if (ifs.fail()) {
        fprintf(stderr, "error: Open file '%s' failed.\n", file.c_str());
        return false;
    }

    ifs.seekg(0, ifs.end);
    auto size = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    size_t buf_size = static_cast<size_t>(size);
    auto buf = new char[buf_size];
    ifs.read(buf, size);
    m_data = std::string(buf, buf_size);
    delete [] buf;
    return true;
}

bool HeapAnalyzer::Analyze(bool verbose)
{
    if (!m_hprof.Parse(m_data, verbose)) {
        return false;
    }

    AnalyzeObject();
    AnalyzeThread();

    return true;
}

void HeapAnalyzer::ShowThread()
{
    printf("Object/Stack Frame                                                            "
        "Shallow Heap   Retained Heap\n");
    printf("============================================================================  "
        "=============  =============\n");
    for (auto thread : m_threads) {
        printf("%s\n", thread.name.c_str());
        for (auto frame : thread.frames) {
            printf("  at %s (%s:%d)\n", frame.name.c_str(), frame.fileName.c_str(), frame.line);
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
}

void HeapAnalyzer::ShowObject()
{
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
}

void HeapAnalyzer::ShowReference(const std::string &objNameList, int maxDepth, bool incoming)
{
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

    auto locals = m_hprof.GetLocalsRoots();
    auto globals = m_hprof.GetGlobalsRoots();
    auto unknown = m_hprof.GetUnknownRoots();

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
}

void HeapAnalyzer::AnalyzeObject()
{
    AnalyzeInstance();
    AnalyzeArray();

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

    /* Sorts objects in lexicographic order by object name. */
    auto comp = [](const std::shared_ptr<Object> &a, const std::shared_ptr<Object> &b) {
        return a->name == b->name ? a->id < b->id : a->name < b->name;
    };
    std::sort(m_objects.begin(), m_objects.end(), comp);
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

void HeapAnalyzer::AnalyzeInstance()
{
    auto strings = m_hprof.GetStrings();
    auto classes = m_hprof.GetClasses();
    for (auto inst : m_hprof.GetInstances()) {
        auto obj = GetObject(inst.first);
        auto cls = classes[inst.second.cls];
        obj->id = inst.first;
        obj->name = ReplaceTypeName(strings[cls.name]);
        obj->size = Align(cls.size);
        for (auto id : inst.second.fields) {
            if (id == 0) {
                continue;
            }

            if (std::count(obj->outRef.begin(), obj->outRef.end(), id) != 0) {
                continue;
            }

            obj->outRef.emplace(id);
            auto ref = GetObject(id);
            ref->inRef.emplace(obj->id);
        }
    }
}

void HeapAnalyzer::AnalyzeArray()
{
    auto strings = m_hprof.GetStrings();
    auto classes = m_hprof.GetClasses();
    auto componentNums = m_hprof.GetComponentNums();
    std::unordered_map<Hprof::BasicType, std::tuple<std::string, int>> typeInfo = {
        { Hprof::BasicType::BOOLEAN, { "RawArray<Byte>[]", 1 } },
        { Hprof::BasicType::SHORT, { "RawArray<Harf>[]", 2 } },
        { Hprof::BasicType::INT, { "RawArray<Word>[]", 4 } },
        { Hprof::BasicType::LONG, { "RawArray<DWord>[]", 8 } }
    };

    for (auto arr : m_hprof.GetArrays()) {
        auto obj = GetObject(arr.first);
        obj->id = arr.first;
        obj->size = arr.second.GetFixedSize();

        if (arr.second.type != Hprof::BasicType::OBJECT) {
            obj->name = std::get<0>(typeInfo[arr.second.type]);
            obj->size += Align(uint64_t(std::get<1>(typeInfo[arr.second.type])) * arr.second.num);
            continue;
        }

        auto cls = classes[arr.second.cls];
        obj->name = ReplaceTypeName(strings[cls.name]);
        obj->size += cls.size != 0 ?
            Align(uint64_t(cls.size) * (componentNums.find(obj->id) != componentNums.end() ?
                componentNums[obj->id]
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
    auto strings = m_hprof.GetStrings();
    auto frames = m_hprof.GetFrames();
    std::map<Hprof::u4, std::vector<Frame>> stackTraces;
    for (auto stackTrace : m_hprof.GetStackTraces()) {
        for (auto frame : stackTrace.second.frames) {
            stackTraces[stackTrace.first].push_back(
                { strings[frames[frame].name], strings[frames[frame].fileName], int(frames[frame].line) }
            );
        }
    }

    std::map<Hprof::u4, Thread> threads;
    for (auto thread : m_hprof.GetThreads()) {
        threads[thread.second.idx] = { strings[thread.second.name], stackTraces[thread.second.stackTraceIdx] };
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
