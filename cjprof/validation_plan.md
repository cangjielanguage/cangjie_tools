# 迁移验证计划

## 1. 编译验证

```bash
mkdir build && cd build
cmake ..
make -j
```

- 验证 cjprof 可执行文件编译成功，无报错、无 warning
- 验证 Windows/Linux 双平台均通过（如环境允许）

## 2. 命令行参数验证

```bash
# 帮助信息包含 --dump-report
cjprof heap --help

# 默认端口启动
cjprof heap -i heap.data --dump-report

# 指定端口启动
cjprof heap -i heap.data --dump-report=9000
```

- 验证参数解析不报错
- 验证默认端口 8080，被占用时自动递增

## 3. Server 启动验证

执行命令后终端应输出：
```
========================================
  Access URL: http://localhost:8080
  Press Ctrl+C to stop
========================================
```

- 验证 server 进程未崩溃
- 验证端口确实在监听（`netstat -an | grep 8080` 或 `curl`）

## 4. API 端点验证（curl / 浏览器）

用 curl 逐个测试 untitled 的所有 API：

```bash
curl http://localhost:8080/api/snapshot
curl http://localhost:8080/api/dominance/tree
curl http://localhost:8080/api/dominance/children?parent_id=xxx
curl http://localhost:8080/api/dominance/top10
curl http://localhost:8080/api/fragment/overview
curl http://localhost:8080/api/fragment/layout
curl http://localhost:8080/api/fragment/summary
```

- 验证返回 HTTP 200 + 有效 JSON
- 验证 JSON 字段与 untitled 输出格式一致
- 验证 `/api/dominance/tree` 的 `nodes` 包含 `id`, `class_name`, `retained_size`, `parent_id`, `depth` 等字段

## 5. 前端页面验证

浏览器打开 `http://localhost:8080`：

- **Dominance Tree 标签页**：
  - Snapshot Overview 区域显示 Objects / GC Roots / Used Size
  - Sunburst 图正常渲染（有颜色分区、hover tooltip）
  - Tree 视图可点击展开/折叠节点
  - Top10 表格按 retained_size 降序排列

- **Memory Fragment 标签页**：
  - 显示 Used / Heap Limit / Utilization 三个统计值
  - 利用率进度条宽度正确
  - 内存布局网格按 Category 正确分组显示

## 6. 与 untitled 的行为对比验证

对同一个 `heap.data` 文件，同时启动 untitled 和迁移后的 cjprof（不同端口）：

```bash
# untitled
./untitled_cjprof heap -i heap.data        # 端口 8080

# 迁移后的 cjprof
cjprof heap -i heap.data --dump-report=8081
```

对比：
- `/api/snapshot` 的 `object_count` / `used_size` 是否一致
- `/api/dominance/top10` 的前 10 条数据是否一致
- `/api/fragment/layout` 的各 Category size 是否一致

## 7. 回归验证

验证 `--dump-report` 不影响现有 heap 命令功能：

```bash
# 现有功能不受影响
cjprof heap -i heap.data              # 正常输出 CLI 表格
cjprof heap -i heap.data -t           # 正常输出线程栈
cjprof heap -i heap.data --show-reference
```

---

*以上为完整验证方案，请确认后执行。*
