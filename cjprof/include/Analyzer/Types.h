#ifndef CJPROF_PROFILE_TYPES_H
#define CJPROF_PROFILE_TYPES_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace cjprof {

// Record tags
namespace RecordTag {
    constexpr uint8_t STRING = 0x01;
    constexpr uint8_t LOAD_CLASS = 0x02;
    constexpr uint8_t STACK_FRAME = 0x04;
    constexpr uint8_t STACK_TRACE = 0x05;
    constexpr uint8_t THREAD = 0x0a;
    constexpr uint8_t HEAP_DUMP = 0x0c;
}

// Heap dump sub-record tags
namespace HeapDumpTag {
    constexpr uint8_t ROOT_UNKNOWN = 0xff;
    constexpr uint8_t ROOT_GLOBAL = 0x01;
    constexpr uint8_t ROOT_LOCAL = 0x02;
    constexpr uint8_t CLASS_DUMP = 0x20;
    constexpr uint8_t INSTANCE_DUMP = 0x21;
    constexpr uint8_t OBJECT_ARRAY_DUMP = 0x22;
    constexpr uint8_t PRIMITIVE_ARRAY_DUMP = 0x23;
    constexpr uint8_t STRUCT_ARRAY_DUMP = 0x24;
    constexpr uint8_t PINNED_INSTANCE_DUMP = 0x25;
    constexpr uint8_t LARGE_INSTANCE_DUMP = 0x26;
    constexpr uint8_t LARGE_OBJECT_ARRAY_DUMP = 0x27;
    constexpr uint8_t LARGE_PRIMITIVE_ARRAY_DUMP = 0x28;
    constexpr uint8_t LARGE_STRUCT_ARRAY_DUMP = 0x29;
    constexpr uint8_t UNMOVABLE_INSTANCE_DUMP = 0x2A;
    constexpr uint8_t UNMOVABLE_OBJECT_ARRAY_DUMP = 0x2B;
    constexpr uint8_t UNMOVABLE_PRIMITIVE_ARRAY_DUMP = 0x2C;
    constexpr uint8_t UNMOVABLE_STRUCT_ARRAY_DUMP = 0x2D;
}

// Object category
enum class ObjectCategory : uint8_t {
    INSTANCE_OBJECT = 0,
    OBJECT_ARRAY = 1,
    STRUCT_ARRAY = 2,
    PRIMITIVE_ARRAY = 3,
    PINNED_OBJECT = 4,
    LARGE_OBJECT = 5,
    UNMOVABLE_OBJECT = 6
};

// GC Root type
enum class RootType : uint8_t {
    UNKNOWN = 0xff,
    GLOBAL = 0x01,
    LOCAL = 0x02
};

// GC Root
struct GcRoot {
    uint64_t object_id;
    RootType type;
    uint32_t thread_idx = 0;
    uint32_t frame_idx = 0;
};

// Class information
struct ClassInfo {
    uint64_t class_id;
    uint64_t name_id;
    std::string class_name;
    uint64_t size = 0;
    bool is_struct = false;
};

// Heap object
struct HeapObject {
    uint64_t object_id;
    uint64_t class_id = 0;
    uint64_t size = 0;
    uint64_t address = 0;
    uint64_t retained_size = 0;
    std::vector<uint64_t> refs;
    ObjectCategory category = ObjectCategory::INSTANCE_OBJECT;
    bool is_pinned = false;
    bool is_large = false;
    bool is_unmovable = false;
};

// Dominance tree node
struct DominanceNode {
    uint64_t object_id;
    uint64_t retained_size = 0;
    uint64_t shallow_size = 0;
    uint32_t depth = 0;
    uint64_t parent_id = 0;
    std::vector<uint64_t> children;
    bool is_class_clustered = false;
    uint32_t instance_count = 1;
};

// Memory fragment
struct MemoryFragment {
    uint64_t start_address;
    uint64_t end_address;
    uint64_t size;
    ObjectCategory category;
    std::optional<uint64_t> object_id;
};

// Snapshot metadata
struct SnapshotInfo {
    uint64_t heap_total_size = 0;
    uint64_t object_count = 0;
    uint64_t gc_root_count = 0;
    uint64_t used_size = 0;
    uint64_t address_range_start = 0;
    uint64_t address_range_end = 0;
};

// String table: string_id -> string content
using StringTable = std::unordered_map<uint64_t, std::string>;

} // namespace cjprof

#endif // CJPROF_PROFILE_TYPES_H
