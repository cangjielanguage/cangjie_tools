# cjprof heap --dump-report 功能迁移计划（最终版）

## 1. 目标

通过 `cjprof heap -i heap.data --dump-report` 命令，**原封不动地迁移** `untitled` 项目的 HTTP Server + SPA 架构，提供：
- **支配树分析**（Sunburst 图、可折叠树、Top10 表格）
- **内存碎片分析**（统计卡片、利用率进度条、内存布局网格）

**明确排除**：不生成静态 HTML 文件，不将数据内嵌到 HTML，不改为纯前端离线方案。严格保留 HTTP Server 运行态 + 前端 fetch API 调用后端的交互模式。

要求：**尽可能复用当前 cjprof 项目已有的解析内存快照、计算支配树等能力，对本项目自身改动最小。**

---

## 2. 架构决策

**严格保留 `untitled` 的 HTTP Server + SPA 架构，不作任何架构变更。**

- 后端：原始 socket 实现的 HTTP 服务器，提供 REST API（JSON）
- 前端：`index.html` 通过 `fetch()` 调用后端 API，D3.js 实时渲染
- 命令行为：执行 `--dump-report` 后，解析数据 → 启动 HTTP Server → 阻塞等待 → 用户在浏览器中打开 `http://localhost:8080` 查看

---

## 3. 从 `untitled` 迁移的文件清单

以下文件从 `untitled` **原封不动迁移**到 `src/Analyzer/` 和 `include/Analyzer/`（仅做路径适配和去除外部依赖）：

| 原路径 (untitled) | 迁移后路径 (cjprof) | 说明 |
|-------------------|---------------------|------|
| `backend/src/profile/types.h` | `include/Analyzer/Types.h` | `HeapObject`, `DominanceNode`, `ClassInfo`, `SnapshotInfo`, `GcRoot` 等类型定义 |
| `backend/src/http/context.h` | `include/Analyzer/HttpContext.h` | HTTP Handler 共享上下文 |
| `backend/src/http/handlers.h` | `include/Analyzer/HttpHandlers.h` | HTTP API 处理器声明 |
| `backend/src/http/handlers.cpp` | `src/Analyzer/HttpHandlers.cpp` | REST API JSON 生成逻辑（原封不动） |
| `backend/src/http/server.h` | `include/Analyzer/HttpServer.h` | HTTP Server 声明 |
| `backend/src/http/server.cpp` | `src/Analyzer/HttpServer.cpp` | 原始 socket HTTP 服务器实现 |
| `backend/static/html/index.html` | `static/html/index.html` | D3.js 前端单页应用（原封不动） |
| `backend/include/spdlog/` | `include/spdlog/` | spdlog 头文件-only 日志库（原封不动复制整个目录） |
| `backend/src/utils/logger.h` | `include/Analyzer/Logger.h` | 原封不动迁移（依赖 spdlog） |
| `backend/src/utils/logger.cpp` | `src/Analyzer/Logger.cpp` | 原封不动迁移 |

**不迁移的文件**：
- `backend/src/dominance/tree.h`, `tree.cpp` → **复用 cjprof 本项目已有的支配树计算能力**（`Cjprof.cpp` 中的 Cooper 算法），不迁移 untitled 的 DominanceTreeBuilder
- `backend/src/profile/parser.h`, `parser.cpp` → **复用 cjprof 的 `Hprof` + `HeapAnalyzer`**
- `backend/src/database/*` → untitled 的 main.cpp 中虽被 include 但实际未使用（缓存逻辑为空）
- `backend/src/main.cpp`, `cli.h`, `cli.cpp` → **复用 cjprof 的 `Heap` 命令入口**
- `backend/src/test_tree.cpp`, `test_http.cpp` → 测试文件，无需迁移

---

## 4. 对当前 cjprof 的改动清单

### 4.1 修改 `include/Data/Hprof.h` / `src/Data/Hprof.cpp`（最小扩展，约 +30 行）

**必要性**：`untitled` 的 `HeapObject` 包含 `category` 字段（`INSTANCE_OBJECT` / `PINNED_OBJECT` / `LARGE_OBJECT` 等），用于内存碎片分析。当前 cjprof 的 `Hprof` 解析器将 `PINNED_INSTANCE_DUMP`、`LARGE_INSTANCE_DUMP`、`UNMOVABLE_INSTANCE_DUMP` 等全部路由到同一解析函数，**丢失了分类信息**。

**改动内容**：
1. 在 `Hprof` 类中新增 `ObjectCategory` 枚举（与 `untitled` 的枚举值完全对齐）
2. 新增 `std::unordered_map<ID, ObjectCategory> m_objectCategories` 和 `GetObjectCategories()` getter
3. 修改 `ParseHeapDumpInstanceDump`、`ParseHeapDumpObjectArrayDump`、`ParseHeapDumpPrimitiveArrayDump`、`ParseHeapDumpStructArrayDump`，增加 `ObjectCategory` 参数（默认 `INSTANCE_OBJECT`）
4. 在 `ParseHeapDump` 的路由 map 中，为 `PINNED_*`、`LARGE_*`、`UNMOVABLE_*` 标签显式传入对应 category

**风险**：极低。仅增加 map 写入，不影响原有解析逻辑和数据结构。

### 4.2 修改 `include/Analyzer/HeapAnalyzer.h` / `src/Analyzer/HeapAnalyzer.cpp`（新增适配层，约 +200 行）

**目标**：将 cjprof 解析后的内部数据，转换为 `untitled` 的 `HttpContext` 所需格式，复用本项目的支配树计算能力，并启动 Report Server。

**改动内容**：
1. `HeapAnalyzer::Object` 增加 `uint8_t category = 0;`（从 `m_hprof.GetObjectCategories()` 读取）
2. 在 `AnalyzeInstance()` / `AnalyzeArray()` 中填充 `category`
3. 新增 private/static 方法：`BuildDominanceNodes()`
   - 基于 `GetRawHeapSnapshot()` 返回的 `RawHeapSnapshot`，复用 `Cjprof.cpp` 中 `RawHeapSnapshotData` 的 Cooper 算法逻辑
   - 计算每个节点的：`retained_size`, `depth`, `parent_id`
   - 输出 `std::vector<DominanceNode>`（与 untitled 格式兼容）
4. 新增 public 方法：`bool StartReportServer(int port = 8080);`
   - 从 `m_hprof` / `m_objects` 构建 `std::vector<HeapObject>`、`std::vector<ClassInfo>`、`std::vector<GcRoot>`、`SnapshotInfo`
   - 调用 `BuildDominanceNodes()` 计算支配树
   - 填充 `HttpContext`
   - 创建 `HttpServer`，`setContext()`，`start()`
   - 打印访问 URL，阻塞等待（`while(true) sleep`）

**复用点**：
- 解析完全复用 `HeapAnalyzer::SetData()` + `Analyze()`
- 对象图引用关系复用 `HeapAnalyzer::Object::outRef/inRef`
- 支配树计算复用本项目 `Cjprof.cpp` 中的 Cooper 算法（改编到 `HeapAnalyzer.cpp` 中，不新增文件）

### 4.3 修改 `include/Commands/Heap.h` / `src/Commands/Heap.cpp`（约 +20 行）

**改动内容**：
1. `Heap.h`：`m_cfg` 增加 `bool dumpReport = false; int reportPort = 8080;`
2. `Heap.cpp` `ParseArgs`：增加 `--dump-report` 长选项（可带可选端口号，如 `--dump-report=9000`）
3. `Heap.cpp` `Execute`：若 `m_cfg.dumpReport` 为 true，在 `Analyze()` 后调用 `analyzer.StartReportServer(m_cfg.reportPort)`
4. `Heap.cpp` `PrintHelp`：增加帮助文本

### 4.4 修改构建系统

**`src/Analyzer/CMakeLists.txt`**（修改）：
```cmake
add_library(Analyzer OBJECT
    HeapAnalyzer.cpp
    HttpHandlers.cpp
    HttpServer.cpp
    Logger.cpp
)
set_target_properties(Analyzer PROPERTIES POSITION_INDEPENDENT_CODE ON)
```

**`src/CMakeLists.txt`**：
- 当前非 Linux 平台未 `add_subdirectory(Reporter)`，但本次迁移**不新增 Reporter 文件**，所有新增代码都在 `Analyzer` 目录下
- `src/Analyzer/CMakeLists.txt` 已在所有平台被包含，无需修改 `src/CMakeLists.txt`

**前端文件部署**：
- `static/html/index.html` 需要在构建后复制到输出目录
- 在根 `CMakeLists.txt` 或 `src/CMakeLists.txt` 中增加 POST_BUILD 复制命令

---

## 5. 数据流

```
heap.data
    │
    ▼
┌──────────────────────────┐   复用现有
│ HeapAnalyzer::SetData()  │
│ HeapAnalyzer::Analyze()  │
└──────────────────────────┘
    │
    ▼
┌──────────────────────────────────────┐   新增适配层
│ HeapAnalyzer::StartReportServer()    │
│  - 构建 HeapObject[] / ClassInfo[]   │
│  - 构建 GcRoot[]                     │
│  - BuildDominanceNodes()             │   复用本项目 Cooper 算法
│  - 填充 HttpContext                  │
│  - HttpServer::start()               │   迁移自 untitled
└──────────────────────────────────────┘
    │
    ▼
http://localhost:8080  ←─ 浏览器打开
    │
    ▼
/static/html/index.html  ←─ 原封不动迁移自 untitled
    │ fetch /api/*
    ▼
HttpHandlers::handle*()  ←─ 原封不动迁移自 untitled
```

---

## 6. 关键技术细节

### 6.1 ObjectCategory 映射

基于 `HeapDumpSubTag` 到 `ObjectCategory` 的映射（与 untitled 文件格式一致）：

| Tag | Category 值 | Category 名称 |
|-----|------------|---------------|
| `INSTANCE_DUMP` (0x21) | 0 | INSTANCE_OBJECT |
| `OBJECT_ARRAY_DUMP` (0x22) | 1 | OBJECT_ARRAY |
| `STRUCT_ARRAY_DUMP` (0x24) | 2 | STRUCT_ARRAY |
| `PRIMITIVE_ARRAY_DUMP` (0x23) | 3 | PRIMITIVE_ARRAY |
| `PINNED_INSTANCE_DUMP` (0x25) | 4 | PINNED_OBJECT |
| `LARGE_INSTANCE_DUMP` (0x26) | 5 | LARGE_OBJECT |
| `LARGE_OBJECT_ARRAY_DUMP` (0x27) | 5 | LARGE_OBJECT |
| `LARGE_PRIMITIVE_ARRAY_DUMP` (0x28) | 5 | LARGE_OBJECT |
| `LARGE_STRUCT_ARRAY_DUMP` (0x29) | 5 | LARGE_OBJECT |
| `UNMOVABLE_INSTANCE_DUMP` (0x2A) | 6 | UNMOVABLE_OBJECT |
| `UNMOVABLE_OBJECT_ARRAY_DUMP` (0x2B) | 6 | UNMOVABLE_OBJECT |
| `UNMOVABLE_PRIMITIVE_ARRAY_DUMP` (0x2C) | 6 | UNMOVABLE_OBJECT |
| `UNMOVABLE_STRUCT_ARRAY_DUMP` (0x2D) | 6 | UNMOVABLE_OBJECT |

### 6.2 路径适配

`HttpServer.cpp` 中的静态文件路径从：
```cpp
std::ifstream file("backend/static/html/index.html");
```
改为：
```cpp
std::ifstream file("static/html/index.html");
```

### 6.3 spdlog 依赖迁移

`untitled` 使用 spdlog 作为头文件-only 日志库。将 `backend/include/spdlog/` 整个目录原封不动复制到当前项目的 `include/spdlog/`。

由于当前项目根 `CMakeLists.txt` 已将 `include` 加入 `include_directories`，`#include "spdlog/spdlog.h"` 可直接找到，无需额外配置。`Logger.h` / `Logger.cpp` 原封不动迁移，不作任何修改。

### 6.4 端口选择

与 `untitled` 的 `main.cpp` 行为一致：默认 8080，若被占用则自动递增直到找到可用端口。

---

## 7. 预期命令行为

```bash
# 默认端口 8080
cjprof heap -i heap.data --dump-report

# 指定端口
cjprof heap -i heap.data --dump-report=9000
```

输出：
```
========================================
  Access URL: http://localhost:8080
  Press Ctrl+C to stop
========================================
```

浏览器打开后：
- **Dominance Tree** 标签页：Snapshot Overview、Sunburst 图、Tree 视图、Top10 表格
- **Memory Fragment** 标签页：Used / Heap Limit / Utilization 统计、利用率进度条、按 Category 汇总的内存布局网格

---

*计划整理完毕，请审阅后确认或提出修改意见，我再开始编码。*
