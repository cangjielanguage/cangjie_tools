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
    m_fileSize = buf_size;
    return true;
}

bool HeapAnalyzer::Analyze(bool verbose)
{
    if (!m_hprof.Parse(m_data, verbose)) {
        return false;
    }

    AnalyzeObject();
    AnalyzeThread();
    FilterPlaceholderObjects();

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
    std::unordered_map<Hprof::ID, Hprof::Local>& locals,
    std::unordered_set<Hprof::ID>& globals,
    std::unordered_set<Hprof::ID>& unknown,
    std::unordered_map<Hprof::ID, Hprof::Array>& arrays)
{
    if (arrays.find(node.id) != arrays.end()) {
        node.type = RawHeapSnapshot::NodeType::ARRAY;
        auto arr = arrays[node.id];
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

    auto locals = m_hprof.GetLocalsRoots();
    auto globals = m_hprof.GetGlobalsRoots();
    auto unknown = m_hprof.GetUnknownRoots();
    auto arrays = m_hprof.GetArrays();

    RawHeapSnapshot data = {};
    for (auto obj : m_objects) {
        RawHeapSnapshot::Node node = {};
        node.id = obj->id;
        GetRawHeapNodeType(node, locals, globals, unknown, arrays);
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
