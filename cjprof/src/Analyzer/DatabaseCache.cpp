#include "Analyzer/DatabaseCache.h"
#include "Analyzer/Logger.h"
#include <fstream>
#include <cstring>
#include <unordered_map>

#ifdef _WIN32
#include <process.h>
#define CJPROF_GETPID() _getpid()
#else
#include <unistd.h>
#define CJPROF_GETPID() getpid()
#endif

namespace cjprof {

#pragma pack(push, 1)
struct BinaryHeader {
    char magic[4] = {'C', 'J', 'D', 'B'};
    uint32_t version = 1;
    uint32_t header_size;

    uint64_t snapshot_offset;
    uint64_t snapshot_size;

    uint64_t classes_offset;
    uint64_t classes_count;

    uint64_t objects_offset;
    uint64_t objects_count;

    uint64_t refs_offset;
    uint64_t refs_count;

    uint64_t gcroots_offset;
    uint64_t gcroots_count;

    uint64_t dominance_offset;
    uint64_t dominance_count;

    uint64_t children_offset;
    uint64_t children_count;

    uint64_t strings_offset;
    uint64_t strings_size;
};

struct SnapshotEntry {
    uint64_t heap_total_size;
    uint64_t object_count;
    uint64_t gc_root_count;
    uint64_t used_size;
};

struct ClassEntry {
    uint64_t class_id;
    uint32_t name_offset;
    uint64_t size;
    uint8_t is_struct;
};

struct ObjectEntry {
    uint64_t object_id;
    uint64_t class_id;
    uint64_t size;
    uint64_t address;
    uint8_t category;
    uint32_t name_offset;
    uint8_t is_pinned;
    uint8_t is_large;
    uint64_t refs_offset;
    uint32_t refs_count;
};

struct GcRootEntry {
    uint8_t type;
    uint64_t object_id;
    uint32_t thread_idx;
    uint32_t frame_idx;
};

struct DominanceEntry {
    uint64_t object_id;
    uint64_t parent_id;
    uint64_t retained_size;
    uint64_t shallow_size;
    uint32_t depth;
    uint64_t children_offset;
    uint32_t children_count;
};
#pragma pack(pop)

static std::string getCachePath(const std::string& heapFilePath) {
    return heapFilePath + ".cjprof.db";
}

bool DatabaseCache::isCacheValid(const std::string& heapFilePath) {
    std::ifstream file(getCachePath(heapFilePath), std::ios::binary);
    if (!file) return false;
    char magic[4];
    file.read(magic, 4);
    return file.gcount() == 4 && std::memcmp(magic, "CJDB", 4) == 0;
}

bool DatabaseCache::save(const std::string& heapFilePath,
                         const SnapshotInfo& snapshot,
                         const std::vector<ClassInfo>& classes,
                         const std::vector<HeapObject>& objects,
                         const std::vector<GcRoot>& gcRoots,
                         const std::vector<DominanceNode>& dominanceNodes)
{
    std::string dbPath = getCachePath(heapFilePath);
    std::string tmpPath = dbPath + ".tmp." + std::to_string(CJPROF_GETPID());
    std::remove(tmpPath.c_str());

    // Build string table (deduplicated)
    std::unordered_map<std::string, uint32_t> stringOffsets;
    std::string stringTableData;
    auto addString = [&](const std::string& s) -> uint32_t {
        auto it = stringOffsets.find(s);
        if (it != stringOffsets.end()) return it->second;
        uint32_t offset = static_cast<uint32_t>(stringTableData.size());
        stringOffsets[s] = offset;
        stringTableData += s;
        stringTableData.push_back('\0');
        return offset;
    };

    // Flatten refs and children into contiguous arrays
    std::vector<uint64_t> flatRefs;
    std::vector<uint64_t> flatChildren;
    size_t totalRefs = 0;
    size_t totalChildren = 0;
    for (const auto& obj : objects) totalRefs += obj.refs.size();
    for (const auto& node : dominanceNodes) totalChildren += node.children.size();
    flatRefs.reserve(totalRefs);
    flatChildren.reserve(totalChildren);
    for (const auto& obj : objects) {
        flatRefs.insert(flatRefs.end(), obj.refs.begin(), obj.refs.end());
    }
    for (const auto& node : dominanceNodes) {
        flatChildren.insert(flatChildren.end(), node.children.begin(), node.children.end());
    }

    // Calculate section offsets
    BinaryHeader header;
    header.header_size = sizeof(BinaryHeader);

    uint64_t offset = sizeof(BinaryHeader);
    header.snapshot_offset = offset;
    header.snapshot_size = sizeof(SnapshotEntry);
    offset += header.snapshot_size;

    header.classes_offset = offset;
    header.classes_count = classes.size();
    offset += classes.size() * sizeof(ClassEntry);

    header.objects_offset = offset;
    header.objects_count = objects.size();
    offset += objects.size() * sizeof(ObjectEntry);

    header.refs_offset = offset;
    header.refs_count = flatRefs.size();
    offset += flatRefs.size() * sizeof(uint64_t);

    header.gcroots_offset = offset;
    header.gcroots_count = gcRoots.size();
    offset += gcRoots.size() * sizeof(GcRootEntry);

    header.dominance_offset = offset;
    header.dominance_count = dominanceNodes.size();
    offset += dominanceNodes.size() * sizeof(DominanceEntry);

    header.children_offset = offset;
    header.children_count = flatChildren.size();
    offset += flatChildren.size() * sizeof(uint64_t);

    header.strings_offset = offset;
    header.strings_size = stringTableData.size();

    // Write file
    std::ofstream file(tmpPath, std::ios::binary);
    if (!file) {
        LOG_ERROR("Failed to open cache file for writing: {}", tmpPath);
        return false;
    }

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    SnapshotEntry snap{snapshot.heap_total_size, snapshot.object_count, snapshot.gc_root_count, snapshot.used_size};
    file.write(reinterpret_cast<const char*>(&snap), sizeof(snap));

    for (const auto& cls : classes) {
        ClassEntry e{cls.class_id, addString(cls.class_name), cls.size, static_cast<uint8_t>(cls.is_struct ? 1 : 0)};
        file.write(reinterpret_cast<const char*>(&e), sizeof(e));
    }

    uint64_t refsIdx = 0;
    for (const auto& obj : objects) {
        ObjectEntry e{
            obj.object_id, obj.class_id, obj.size, obj.address,
            static_cast<uint8_t>(obj.category), addString(obj.name),
            static_cast<uint8_t>(obj.is_pinned ? 1 : 0), static_cast<uint8_t>(obj.is_large ? 1 : 0),
            refsIdx, static_cast<uint32_t>(obj.refs.size())
        };
        file.write(reinterpret_cast<const char*>(&e), sizeof(e));
        refsIdx += obj.refs.size();
    }

    if (!flatRefs.empty()) {
        file.write(reinterpret_cast<const char*>(flatRefs.data()), flatRefs.size() * sizeof(uint64_t));
    }

    for (const auto& root : gcRoots) {
        GcRootEntry e{static_cast<uint8_t>(root.type), root.object_id, root.thread_idx, root.frame_idx};
        file.write(reinterpret_cast<const char*>(&e), sizeof(e));
    }

    uint64_t childIdx = 0;
    for (const auto& node : dominanceNodes) {
        DominanceEntry e{
            node.object_id, node.parent_id, node.retained_size, node.shallow_size,
            node.depth, childIdx, static_cast<uint32_t>(node.children.size())
        };
        file.write(reinterpret_cast<const char*>(&e), sizeof(e));
        childIdx += node.children.size();
    }

    if (!flatChildren.empty()) {
        file.write(reinterpret_cast<const char*>(flatChildren.data()), flatChildren.size() * sizeof(uint64_t));
    }

    if (!stringTableData.empty()) {
        file.write(stringTableData.data(), stringTableData.size());
    }

    file.close();

    // Atomic rename
    std::remove(dbPath.c_str());
    if (std::rename(tmpPath.c_str(), dbPath.c_str()) != 0) {
        LOG_ERROR("Failed to rename cache file from {} to {}", tmpPath, dbPath);
        std::remove(tmpPath.c_str());
        return false;
    }

    LOG_DEBUG("Binary cache saved: {} (objects={}, refs={}, children={})",
        dbPath, objects.size(), flatRefs.size(), flatChildren.size());
    return true;
}

bool DatabaseCache::load(const std::string& heapFilePath,
                         SnapshotInfo& snapshot,
                         std::vector<ClassInfo>& classes,
                         std::vector<HeapObject>& objects,
                         std::vector<GcRoot>& gcRoots,
                         std::vector<DominanceNode>& dominanceNodes,
                         StringTable& stringTable)
{
    std::string dbPath = getCachePath(heapFilePath);
    std::ifstream file(dbPath, std::ios::binary);
    if (!file) {
        LOG_ERROR("Failed to open cache file: {}", dbPath);
        return false;
    }

    BinaryHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (file.gcount() != sizeof(header)) {
        LOG_ERROR("Cache file too small: {}", dbPath);
        return false;
    }
    if (std::memcmp(header.magic, "CJDB", 4) != 0) {
        LOG_ERROR("Invalid cache file magic: {}", dbPath);
        return false;
    }
    if (header.version != 1) {
        LOG_ERROR("Unsupported cache file version: {}", header.version);
        return false;
    }

    // Read snapshot
    file.seekg(static_cast<std::streamoff>(header.snapshot_offset));
    SnapshotEntry snap;
    file.read(reinterpret_cast<char*>(&snap), sizeof(snap));
    snapshot.heap_total_size = snap.heap_total_size;
    snapshot.object_count = snap.object_count;
    snapshot.gc_root_count = snap.gc_root_count;
    snapshot.used_size = snap.used_size;

    // Read string table
    std::string stringTableData;
    if (header.strings_size > 0) {
        stringTableData.resize(header.strings_size);
        file.seekg(static_cast<std::streamoff>(header.strings_offset));
        file.read(stringTableData.data(), static_cast<std::streamsize>(header.strings_size));
    }
    auto getString = [&](uint32_t offset) -> const char* {
        return offset < stringTableData.size() ? stringTableData.data() + offset : "";
    };

    struct ObjectRefsInfo { uint64_t refs_offset; uint32_t refs_count; };
    struct NodeChildrenInfo { uint64_t children_offset; uint32_t children_count; };

    std::vector<ObjectRefsInfo> objectRefsInfos;
    std::vector<NodeChildrenInfo> nodeChildrenInfos;
    objectRefsInfos.reserve(header.objects_count);
    nodeChildrenInfos.reserve(header.dominance_count);

    // Read classes
    classes.resize(header.classes_count);
    if (header.classes_count > 0) {
        file.seekg(static_cast<std::streamoff>(header.classes_offset));
        for (uint64_t i = 0; i < header.classes_count; ++i) {
            ClassEntry e;
            file.read(reinterpret_cast<char*>(&e), sizeof(e));
            ClassInfo cls;
            cls.class_id = e.class_id;
            cls.class_name = getString(e.name_offset);
            cls.size = e.size;
            cls.is_struct = e.is_struct != 0;
            classes[i] = cls;
            stringTable[e.class_id] = cls.class_name;
        }
    }

    // Read objects
    objects.resize(header.objects_count);
    if (header.objects_count > 0) {
        file.seekg(static_cast<std::streamoff>(header.objects_offset));
        for (uint64_t i = 0; i < header.objects_count; ++i) {
            ObjectEntry e;
            file.read(reinterpret_cast<char*>(&e), sizeof(e));
            HeapObject obj;
            obj.object_id = e.object_id;
            obj.class_id = e.class_id;
            obj.size = e.size;
            obj.address = e.address;
            obj.category = static_cast<ObjectCategory>(e.category);
            obj.name = getString(e.name_offset);
            obj.is_pinned = e.is_pinned != 0;
            obj.is_large = e.is_large != 0;
            objects[i] = obj;
            objectRefsInfos.push_back({e.refs_offset, e.refs_count});
        }
    }

    // Read refs and populate objects.refs
    if (header.refs_count > 0) {
        std::vector<uint64_t> flatRefs(header.refs_count);
        file.seekg(static_cast<std::streamoff>(header.refs_offset));
        file.read(reinterpret_cast<char*>(flatRefs.data()), static_cast<std::streamsize>(header.refs_count * sizeof(uint64_t)));
        for (size_t i = 0; i < objects.size(); ++i) {
            const auto& info = objectRefsInfos[i];
            if (info.refs_count > 0) {
                objects[i].refs.assign(flatRefs.begin() + info.refs_offset, flatRefs.begin() + info.refs_offset + info.refs_count);
            }
        }
    }

    // Read gc roots
    gcRoots.resize(header.gcroots_count);
    if (header.gcroots_count > 0) {
        file.seekg(static_cast<std::streamoff>(header.gcroots_offset));
        for (uint64_t i = 0; i < header.gcroots_count; ++i) {
            GcRootEntry e;
            file.read(reinterpret_cast<char*>(&e), sizeof(e));
            GcRoot root;
            root.type = static_cast<RootType>(e.type);
            root.object_id = e.object_id;
            root.thread_idx = e.thread_idx;
            root.frame_idx = e.frame_idx;
            gcRoots[i] = root;
        }
    }

    // Read dominance nodes
    dominanceNodes.resize(header.dominance_count);
    if (header.dominance_count > 0) {
        file.seekg(static_cast<std::streamoff>(header.dominance_offset));
        for (uint64_t i = 0; i < header.dominance_count; ++i) {
            DominanceEntry e;
            file.read(reinterpret_cast<char*>(&e), sizeof(e));
            DominanceNode node;
            node.object_id = e.object_id;
            node.parent_id = e.parent_id;
            node.retained_size = e.retained_size;
            node.shallow_size = e.shallow_size;
            node.depth = e.depth;
            dominanceNodes[i] = node;
            nodeChildrenInfos.push_back({e.children_offset, e.children_count});
        }
    }

    // Read children and populate dominanceNodes.children
    if (header.children_count > 0) {
        std::vector<uint64_t> flatChildren(header.children_count);
        file.seekg(static_cast<std::streamoff>(header.children_offset));
        file.read(reinterpret_cast<char*>(flatChildren.data()), static_cast<std::streamsize>(header.children_count * sizeof(uint64_t)));
        for (size_t i = 0; i < dominanceNodes.size(); ++i) {
            const auto& info = nodeChildrenInfos[i];
            if (info.children_count > 0) {
                dominanceNodes[i].children.assign(flatChildren.begin() + info.children_offset, flatChildren.begin() + info.children_offset + info.children_count);
            }
        }
    }

    LOG_DEBUG("Binary cache loaded: {} (objects={}, refs={}, children={})",
        dbPath, objects.size(), header.refs_count, header.children_count);
    return true;
}

} // namespace cjprof
