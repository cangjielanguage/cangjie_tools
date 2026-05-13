#include "Analyzer/DatabaseSchema.h"

namespace cjprof {

const char* DatabaseSchema::getCreateTablesSQL() {
    return R"(
        CREATE TABLE IF NOT EXISTS snapshots (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            filepath TEXT NOT NULL UNIQUE,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            heap_total_size INTEGER NOT NULL,
            object_count INTEGER NOT NULL,
            gc_root_count INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS classes (
            id INTEGER PRIMARY KEY,
            snapshot_id INTEGER NOT NULL,
            class_name TEXT NOT NULL,
            size INTEGER NOT NULL,
            is_struct INTEGER NOT NULL,
            is_pinned INTEGER NOT NULL,
            FOREIGN KEY (snapshot_id) REFERENCES snapshots(id)
        );

        CREATE TABLE IF NOT EXISTS objects (
            id INTEGER PRIMARY KEY,
            snapshot_id INTEGER NOT NULL,
            class_id INTEGER,
            size INTEGER NOT NULL,
            address INTEGER NOT NULL,
            category INTEGER NOT NULL,
            name TEXT NOT NULL DEFAULT '',
            is_pinned INTEGER NOT NULL DEFAULT 0,
            is_large INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (snapshot_id) REFERENCES snapshots(id),
            FOREIGN KEY (class_id) REFERENCES classes(id)
        );

        CREATE TABLE IF NOT EXISTS object_refs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            snapshot_id INTEGER NOT NULL,
            from_object_id INTEGER NOT NULL,
            to_object_id INTEGER NOT NULL,
            FOREIGN KEY (snapshot_id) REFERENCES snapshots(id),
            FOREIGN KEY (from_object_id) REFERENCES objects(id),
            FOREIGN KEY (to_object_id) REFERENCES objects(id)
        );

        CREATE TABLE IF NOT EXISTS gc_roots (
            id INTEGER PRIMARY KEY,
            snapshot_id INTEGER NOT NULL,
            root_type INTEGER NOT NULL,
            object_id INTEGER NOT NULL,
            thread_idx INTEGER,
            frame_idx INTEGER,
            FOREIGN KEY (snapshot_id) REFERENCES snapshots(id),
            FOREIGN KEY (object_id) REFERENCES objects(id)
        );

        CREATE TABLE IF NOT EXISTS dominance_tree (
            id INTEGER PRIMARY KEY,
            snapshot_id INTEGER NOT NULL,
            object_id INTEGER NOT NULL,
            parent_id INTEGER,
            retained_size INTEGER NOT NULL,
            shallow_size INTEGER NOT NULL,
            depth INTEGER NOT NULL,
            FOREIGN KEY (snapshot_id) REFERENCES snapshots(id),
            FOREIGN KEY (object_id) REFERENCES objects(id),
            FOREIGN KEY (parent_id) REFERENCES dominance_tree(id)
        );

        CREATE TABLE IF NOT EXISTS memory_fragments (
            id INTEGER PRIMARY KEY,
            snapshot_id INTEGER NOT NULL,
            start_address INTEGER NOT NULL,
            end_address INTEGER NOT NULL,
            size INTEGER NOT NULL,
            category INTEGER NOT NULL,
            object_id INTEGER,
            FOREIGN KEY (snapshot_id) REFERENCES snapshots(id),
            FOREIGN KEY (object_id) REFERENCES objects(id)
        );
    )";
}

const char* DatabaseSchema::getCreateIndexesSQL() {
    return R"(
        CREATE INDEX IF NOT EXISTS idx_classes_snapshot ON classes(snapshot_id);
        CREATE INDEX IF NOT EXISTS idx_objects_snapshot ON objects(snapshot_id);
        CREATE INDEX IF NOT EXISTS idx_objects_address ON objects(address);
        CREATE INDEX IF NOT EXISTS idx_refs_from ON object_refs(snapshot_id, from_object_id);
        CREATE INDEX IF NOT EXISTS idx_refs_to ON object_refs(snapshot_id, to_object_id);
        CREATE INDEX IF NOT EXISTS idx_gc_roots_snapshot ON gc_roots(snapshot_id);
        CREATE INDEX IF NOT EXISTS idx_dominance_snapshot ON dominance_tree(snapshot_id);
        CREATE INDEX IF NOT EXISTS idx_dominance_parent ON dominance_tree(snapshot_id, parent_id);
        CREATE INDEX IF NOT EXISTS idx_fragments_snapshot ON memory_fragments(snapshot_id);
        CREATE INDEX IF NOT EXISTS idx_fragments_address ON memory_fragments(snapshot_id, start_address);
    )";
}

} // namespace cjprof
