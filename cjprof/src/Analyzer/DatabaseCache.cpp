#include "Analyzer/DatabaseCache.h"
#include "Analyzer/SqliteDb.h"
#include "Analyzer/DatabaseSchema.h"
#include "Analyzer/Logger.h"
#include <fstream>

namespace cjprof {

static std::string getCachePath(const std::string& heapFilePath) {
    return heapFilePath + ".cjprof.db";
}

bool DatabaseCache::isCacheValid(const std::string& heapFilePath) {
    std::ifstream dbFile(getCachePath(heapFilePath), std::ios::binary);
    return dbFile.is_open();
}

bool DatabaseCache::save(const std::string& heapFilePath,
                         const SnapshotInfo& snapshot,
                         const std::vector<ClassInfo>& classes,
                         const std::vector<HeapObject>& objects,
                         const std::vector<GcRoot>& gcRoots,
                         const std::vector<DominanceNode>& dominanceNodes)
{
    SQLite db;
    std::string dbPath = getCachePath(heapFilePath);
    if (!db.open(dbPath)) {
        LOG_ERROR("Failed to open database: {}", dbPath);
        return false;
    }

    // Create schema
    if (!db.execute(DatabaseSchema::getCreateTablesSQL())) {
        LOG_ERROR("Failed to create tables");
        return false;
    }
    if (!db.execute(DatabaseSchema::getCreateIndexesSQL())) {
        LOG_ERROR("Failed to create indexes");
        return false;
    }

    // Clear old data for this snapshot (id=1)
    db.execute("DELETE FROM memory_fragments WHERE snapshot_id = 1;");
    db.execute("DELETE FROM dominance_tree WHERE snapshot_id = 1;");
    db.execute("DELETE FROM gc_roots WHERE snapshot_id = 1;");
    db.execute("DELETE FROM object_refs WHERE snapshot_id = 1;");
    db.execute("DELETE FROM objects WHERE snapshot_id = 1;");
    db.execute("DELETE FROM classes WHERE snapshot_id = 1;");
    db.execute("DELETE FROM snapshots WHERE id = 1;");

    // Begin transaction
    if (!db.execute("BEGIN TRANSACTION;")) {
        LOG_ERROR("Failed to begin transaction");
        return false;
    }

    // Insert snapshot
    {
        if (!db.prepare("INSERT INTO snapshots (id, filepath, heap_total_size, object_count, gc_root_count) VALUES (?, ?, ?, ?, ?);")) {
            db.execute("ROLLBACK;");
            return false;
        }
        db.bindInt64(1, 1);
        db.bindText(2, heapFilePath);
        db.bindInt64(3, static_cast<int64_t>(snapshot.heap_total_size));
        db.bindInt64(4, static_cast<int64_t>(snapshot.object_count));
        db.bindInt64(5, static_cast<int64_t>(snapshot.gc_root_count));
        if (!db.stepDone()) {
            db.execute("ROLLBACK;");
            return false;
        }
        db.finalize();
    }

    // Insert classes
    {
        if (!db.prepare("INSERT INTO classes (id, snapshot_id, class_name, size, is_struct, is_pinned) VALUES (?, 1, ?, ?, ?, 0);")) {
            db.execute("ROLLBACK;");
            return false;
        }
        for (const auto& cls : classes) {
            db.bindInt64(1, static_cast<int64_t>(cls.class_id));
            db.bindText(2, cls.class_name);
            db.bindInt64(3, static_cast<int64_t>(cls.size));
            db.bindInt(4, cls.is_struct ? 1 : 0);
            db.stepDone();
            db.reset();
        }
        db.finalize();
    }

    // Insert objects
    {
        if (!db.prepare("INSERT INTO objects (id, snapshot_id, class_id, size, address, category, is_pinned, is_large) VALUES (?, 1, ?, ?, ?, ?, ?, ?);")) {
            db.execute("ROLLBACK;");
            return false;
        }
        for (const auto& obj : objects) {
            db.bindInt64(1, static_cast<int64_t>(obj.object_id));
            db.bindInt64(2, static_cast<int64_t>(obj.class_id));
            db.bindInt64(3, static_cast<int64_t>(obj.size));
            db.bindInt64(4, static_cast<int64_t>(obj.address));
            db.bindInt(5, static_cast<int>(obj.category));
            db.bindInt(6, obj.is_pinned ? 1 : 0);
            db.bindInt(7, obj.is_large ? 1 : 0);
            db.stepDone();
            db.reset();
        }
        db.finalize();
    }

    // Insert object refs
    {
        if (!db.prepare("INSERT INTO object_refs (snapshot_id, from_object_id, to_object_id) VALUES (1, ?, ?);")) {
            db.execute("ROLLBACK;");
            return false;
        }
        for (const auto& obj : objects) {
            for (uint64_t ref : obj.refs) {
                db.bindInt64(1, static_cast<int64_t>(obj.object_id));
                db.bindInt64(2, static_cast<int64_t>(ref));
                db.step();
                db.reset();
            }
        }
        db.finalize();
    }

    // Insert gc roots
    {
        if (!db.prepare("INSERT INTO gc_roots (id, snapshot_id, root_type, object_id, thread_idx, frame_idx) VALUES (?, 1, ?, ?, ?, ?);")) {
            db.execute("ROLLBACK;");
            return false;
        }
        int64_t rootId = 1;
        for (const auto& root : gcRoots) {
            db.bindInt64(1, rootId++);
            db.bindInt(2, static_cast<int>(root.type));
            db.bindInt64(3, static_cast<int64_t>(root.object_id));
            db.bindInt(4, static_cast<int>(root.thread_idx));
            db.bindInt(5, static_cast<int>(root.frame_idx));
            db.stepDone();
            db.reset();
        }
        db.finalize();
    }

    // Insert dominance tree
    {
        if (!db.prepare("INSERT INTO dominance_tree (id, snapshot_id, object_id, parent_id, retained_size, shallow_size, depth) VALUES (?, 1, ?, ?, ?, ?, ?);")) {
            db.execute("ROLLBACK;");
            return false;
        }
        int64_t nodeId = 1;
        for (const auto& node : dominanceNodes) {
            db.bindInt64(1, nodeId++);
            db.bindInt64(2, static_cast<int64_t>(node.object_id));
            db.bindInt64(3, static_cast<int64_t>(node.parent_id));
            db.bindInt64(4, static_cast<int64_t>(node.retained_size));
            db.bindInt64(5, static_cast<int64_t>(node.shallow_size));
            db.bindInt(6, static_cast<int>(node.depth));
            db.stepDone();
            db.reset();
        }
        db.finalize();
    }

    // Commit transaction
    if (!db.execute("COMMIT;")) {
        LOG_ERROR("Failed to commit transaction");
        return false;
    }

    LOG_DEBUG("Database cache saved: {}", dbPath);
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
    SQLite db;
    std::string dbPath = getCachePath(heapFilePath);
    if (!db.open(dbPath)) {
        LOG_ERROR("Failed to open database: {}", dbPath);
        return false;
    }

    // Load snapshot
    {
        if (!db.prepare("SELECT heap_total_size, object_count, gc_root_count FROM snapshots WHERE id = 1;")) {
            return false;
        }
        if (db.step()) {
            snapshot.heap_total_size = static_cast<uint64_t>(db.getColumnInt64(0));
            snapshot.object_count = static_cast<uint64_t>(db.getColumnInt64(1));
            snapshot.gc_root_count = static_cast<uint64_t>(db.getColumnInt64(2));
        }
        db.finalize();
    }

    // Load classes
    {
        if (!db.prepare("SELECT id, class_name, size, is_struct FROM classes WHERE snapshot_id = 1;")) {
            return false;
        }
        while (db.step()) {
            ClassInfo cls;
            cls.class_id = static_cast<uint64_t>(db.getColumnInt64(0));
            cls.class_name = db.getColumnText(1);
            cls.size = static_cast<uint64_t>(db.getColumnInt64(2));
            cls.is_struct = db.getColumnInt(3) != 0;
            classes.push_back(cls);
            // Rebuild string table from classes
            // Note: name_id is not stored in DB design, we use class_id as name_id for reconstruction
            stringTable[cls.class_id] = cls.class_name;
        }
        db.finalize();
    }

    // Load objects
    {
        if (!db.prepare("SELECT id, class_id, size, address, category, is_pinned, is_large FROM objects WHERE snapshot_id = 1;")) {
            return false;
        }
        while (db.step()) {
            HeapObject obj;
            obj.object_id = static_cast<uint64_t>(db.getColumnInt64(0));
            obj.class_id = static_cast<uint64_t>(db.getColumnInt64(1));
            obj.size = static_cast<uint64_t>(db.getColumnInt64(2));
            obj.address = static_cast<uint64_t>(db.getColumnInt64(3));
            obj.category = static_cast<ObjectCategory>(db.getColumnInt(4));
            obj.is_pinned = db.getColumnInt(5) != 0;
            obj.is_large = db.getColumnInt(6) != 0;
            objects.push_back(obj);
        }
        db.finalize();
    }

    // Load object refs and populate objects.refs
    {
        if (!db.prepare("SELECT from_object_id, to_object_id FROM object_refs WHERE snapshot_id = 1;")) {
            return false;
        }
        // Build object_id -> index map for fast lookup
        std::unordered_map<uint64_t, size_t> objIndexMap;
        for (size_t i = 0; i < objects.size(); ++i) {
            objIndexMap[objects[i].object_id] = i;
        }
        while (db.step()) {
            uint64_t fromId = static_cast<uint64_t>(db.getColumnInt64(0));
            uint64_t toId = static_cast<uint64_t>(db.getColumnInt64(1));
            auto it = objIndexMap.find(fromId);
            if (it != objIndexMap.end()) {
                objects[it->second].refs.push_back(toId);
            }
        }
        db.finalize();
    }

    // Load gc roots
    {
        if (!db.prepare("SELECT root_type, object_id, thread_idx, frame_idx FROM gc_roots WHERE snapshot_id = 1;")) {
            return false;
        }
        while (db.step()) {
            GcRoot root;
            root.type = static_cast<RootType>(db.getColumnInt(0));
            root.object_id = static_cast<uint64_t>(db.getColumnInt64(1));
            root.thread_idx = static_cast<uint32_t>(db.getColumnInt(2));
            root.frame_idx = static_cast<uint32_t>(db.getColumnInt(3));
            gcRoots.push_back(root);
        }
        db.finalize();
    }

    // Load dominance tree
    {
        if (!db.prepare("SELECT object_id, parent_id, retained_size, shallow_size, depth FROM dominance_tree WHERE snapshot_id = 1;")) {
            return false;
        }
        while (db.step()) {
            DominanceNode node;
            node.object_id = static_cast<uint64_t>(db.getColumnInt64(0));
            node.parent_id = static_cast<uint64_t>(db.getColumnInt64(1));
            node.retained_size = static_cast<uint64_t>(db.getColumnInt64(2));
            node.shallow_size = static_cast<uint64_t>(db.getColumnInt64(3));
            node.depth = static_cast<uint32_t>(db.getColumnInt(4));
            dominanceNodes.push_back(node);
        }
        db.finalize();
    }

    // Rebuild children for dominance nodes
    {
        std::unordered_map<uint64_t, size_t> nodeIndexMap;
        for (size_t i = 0; i < dominanceNodes.size(); ++i) {
            nodeIndexMap[dominanceNodes[i].object_id] = i;
        }
        for (auto& node : dominanceNodes) {
            if (node.parent_id != 0) {
                auto it = nodeIndexMap.find(node.parent_id);
                if (it != nodeIndexMap.end()) {
                    dominanceNodes[it->second].children.push_back(node.object_id);
                }
            }
        }
    }

    // Calculate used_size from objects
    snapshot.used_size = 0;
    for (const auto& obj : objects) {
        snapshot.used_size += obj.size;
    }

    LOG_DEBUG("Database cache loaded: {} (objects={}, roots={})", dbPath, objects.size(), gcRoots.size());
    return true;
}

} // namespace cjprof
