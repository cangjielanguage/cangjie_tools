# 性能分析工具

## 功能简介

`cjprof（Cangjie Profile）` 是仓颉语言的性能分析工具，支持以下功能：

- 对仓颉语言程序进行 CPU 热点函数采样，导出采样数据。

- 对热点函数采样数据进行分析，生成 CPU 热点函数统计报告或火焰图。

- 导出仓颉语言应用程序堆内存，并对其进行分析生成分析报告。

目前 `cjprof` 在 `Linux` 系统上支持上述功能，在 `macOS` 系统和 `Windows` 系统上仅支持仓颉语言应用程序堆内存分析功能。

## 使用说明

通过 `cjprof --help` 即可查看命令使用方法。支持 `record`，`report` 和 `heap` 子命令，分别用于采集 CPU 热点函数信息， 生成 CPU 热点函数报告（包含火焰图）和导出与分析堆内存。

```text
cjprof --help
 Usage: cjprof [--help] COMMAND [ARGS]

The supported commands are:
  -v        Print version of cjprof
  heap      Dump heap into a dump file or analyze the heap dump file
  record    Run a command and record its profile data into data file
  report    Read profile data file (created by cjprof record) and display the profile
```

> **注意：**
>
> 由于 `cjprof record` 依赖系统的 `perf` 权限，因此使用需要满足以下两个条件之一：
>
> - 使用 `root` 用户或 `sudo` 权限执行。
> - 系统的 `perf_event_paranoid` 参数（通过 `/proc/sys/kernel/perf_event_paranoid` 文件）配置为 -1 。
>
> 否则可能会出现权限不足的问题。

### 采集 CPU 热点函数信息

#### 命令

```text
cjprof record
```

#### 格式

```text
cjprof record [<options>] [<command>]
cjprof record [<options>] -- <command> [<options>]
```

#### 选项

`-f, --freq <freq>` 指定采样频率，单位为赫兹（Hz），即每秒采样次数，默认为 5000 Hz，当指定为 max 或超过系统支持的最大频率时，取系统支持的最大频率。

`-o, --output <file>` 指定采样结束后生成的采样数据文件名，默认为 `cjprof.data` 。

`-p, --pid <pid>` 指定被采样应用程序的进程 ID，当指定 `<command>` 新启动应用程序进行采样时，该选项会被忽略。

#### 示例

- 采样正在运行的应用程序。

```text
# 以 10000 Hz 的采样频率对正在运行的应用程序（进程号为 12345）进行采样，采样结束后将采样数据生成在当前路径下名为 sample.data 的文件中。
cjprof record -f 10000 -p 12345 -o sample.data
```

- 新启动应用程序并对其进行采样。

```text
# 执行当前路径下的 `test` 应用程序，参数为 `arg1 arg2` ，并以系统支持的最大采样频率对其进行采样，采样结束后将采样数据生成在当前路径下名为 `cjprof.data` （默认文件名）的文件中。
cjprof record -f max -- ./test arg1 arg2
```

#### 注意事项

- 开始采样后，只有被采样程序退出后才会结束采样，如果需要提前结束采样，可以在采样过程中通过按 `Ctrl+C` 主动停止采样。

### 生成 CPU 热点函数报告

#### 命令

```text
cjprof report
```

#### 格式

```text
cjprof report [<options>]
```

#### 选项

`-F, --flame-graph` 生成 CPU 热点函数火焰图，而非默认的文本报告。

`-i, --input <file>` 采样数据文件名，默认为 `cjprof.data` 。

`-o, --output <file>` 生成的 CPU 热点函数火焰图文件名，默认为 `FlameGraph.svg`，仅当生成火焰图时才有效。

#### 示例

- 生成默认的 CPU 热点函数文本报告。

```text
# 分析 sample.data 中的采样数据，生成 CPU 热点函数文本报告。
cjprof report -i sample.data
```

- 生成 CPU 热点函数火焰图。

```text
# 分析 cjprof.data（默认文件）中的采样数据，生成名为 test.svg 的 CPU 热点函数火焰图。
cjprof report -F -o test.svg
```

#### 报告形式说明

- 文本形式的报告包含函数采样总占比（包含子函数）、函数采样占比（自身）以及函数名（如果没有对应的符号信息则显示为地址）三部分，报告结果以函数采样总占比降序排列。

- 火焰图中的横轴代表采样占比大小，越宽表示采样占比越大，即运行时间越长，纵轴表示调用栈，父函数在下，子函数在上。

### 导出和分析堆内存

#### 命令

```text
cjprof heap
```

#### 格式

```text
cjprof heap [<options>]
```

#### 选项

`-D, --depth <depth>` 指定对象的引用/被引用关系最大展示深度，默认为 10 层，仅在指定了 `--show-reference` 时才能生效。

`-d, --dump <pid>` 导出仓颉应用程序当前时刻的堆内存，`pid` 为应用程序进程号，当指定为应用程序的子线程号时，同样可导出。

`-i, --input <file>` 指定进行分析的堆内存数据文件名，默认为 `cjprof.data` 。

`-o, --output <file>` 指定导出的堆内存数据文件名，默认为 `cjprof.data` 。

`--show-reference[=<objnames>]` 分析报告中展示对象的引用关系，`objnames` 为需要展示的对象名，多个对象使用 `;` 隔开，不指定时默认展示所有对象。如果该对象是堆的根节点，还会显示对象所属堆的根节点类别。

`--incoming-reference` 展示对象的被引用关系，而非引用关系，需要与 `--show-reference` 配合使用。

`-t, --show-thread` 分析报告中展示仓颉线程栈，以及在栈中引用的对象。

`-V, --verbose` 维测选项，解析堆内存数据文件时打印解析日志。

#### 示例

- 导出堆内存数据。

```text
# 将正在运行的应用程序（进程号为 12345）当前时刻的堆内存导出到当前路径下名为 heap.data 的文件中。
cjprof heap -d 12345 -o heap.data
```

> **注意：**
>
> 导出堆内存时会向进程发送 `SIG_USR1` 信号，在不确定目标进程是否为仓颉应用程序时，需要谨慎操作，否则可能会给目标进程误发送信号导致非预期错误。
> 导出堆内存时，正在运行的仓颉程序目录和执行导出的目录都需要写权限，否则可能因为权限问题导致失败。

- 分析堆内存数据，展示对象信息。

```text
# 解析并分析 ~ 目录下名为 heap.data 的堆内存数据文件，展示堆中各激活对象的对象类型名、实例个数、浅堆大小和深堆大小。
cjprof heap -i ~/heap.data
```

执行上述命令的效果如下：

```text
Object Type           Objects        Shallow Heap   Retained Heap
====================  =============  =============  =============
AAA                               1            80             400
BBB                               4            32             196
CCC                               2            16              32
```

- 分析堆内存数据，展示仓颉线程栈及对象引用。

```text
# 解析并分析当前目录下名为 cjprof.data（默认文件）的堆内存数据文件，展示仓颉线程栈与栈中引用的对象。
cjprof heap --show-thread
```

执行上述命令的效果如下：

```text
Object/Stack Frame                   Shallow Heap   Retained Heap
===================================  =============  =============
thread0
  at Func2() (/home/test/test.cj:10)
    <local> AAA @ 0x7f1234567800                80            400
  at Func1() (/home/test/test.cj:20)
    <local> CCC @ 0x7f12345678c0                16             16
  at main (/home/test/test.cj:30)
```

- 分析堆内存数据，展示对象的引用关系。

```text
# 解析并分析当前目录下名为 cjprof.data（默认文件）的堆内存数据文件，展示 AAA 和 BBB 类型对象的引用关系。
cjprof heap --show-reference="AAA;BBB"
```

执行上述命令的效果如下：

```text
Objects with outgoing references:
Object Type                          Shallow Heap   Retained Heap
===================================  =============  =============
AAA @ 0x7f1234567800                            80            400
  BBB @ 0x7f1234567880                          32             48
    CCC @ 0x7f12345678c0                        16             16
  CCC @ 0x7f12345678e0                          16             16
BBB @ 0x7f1234567880                            32             48
  CCC @ 0x7f12345678c0                          16             16
```

- 分析堆内存数据，展示对象的被引用关系。

```text
# 解析并分析当前目录下名为 cjprof.data（默认文件）的堆内存数据文件，展示 CCC 类型对象的被引用关系。
cjprof heap --show-reference="CCC" --incoming-reference
```

执行上述命令的效果如下：

```text
Objects with incoming references:
Object Type                          Shallow Heap   Retained Heap
===================================  =============  =============
CCC @ 0x7f12345678c0                            16             16
  BBB @ 0x7f1234567880                          32             48
    AAA @ 0x7f1234567800                        80            400
CCC @ 0x7f12345678e0                            16             16
  AAA @ 0x7f1234567800                          80            400
```

#### 堆内存分析报告说明

- 对象类型名使用 `RawArray<Byte>[]`，`RawArray<Half>[]`，`RawArray<Word>[]` 和 `RawArray<DWord>[]` 分别表示 1 字节、2 字节、4 字节和 8 字节大小的基础数据类型原始数组。

- 浅堆（Shallow Heap）是指对象自身所占用的堆内存大小，深堆（Retained Heap）是指对象被垃圾回收后，可以被释放的所有对象（即通过该对象直接或间接引用到的对象）的浅堆大小之和。

- 当对象的引用关系层级超出最大展示深度后，或是存在循环引用出现重复对象后，会使用 `...` 来省略后续引用。

#### 内存分析 API

`cjprof` 动态库提供以下内存分析 API 支持二次开发。

##### 解析内存快照文件

```cpp
bool ParseHeapSnapshotFiles(std::vector<std::string>& filePaths);
```

`ParseHeapSnapshotFiles` 接受以下入参：

- filePaths：内存快照文件路径列表

仅当所有内存快照均解析成功时，返回 `true`。

##### 清理内存快照数据

```cpp
void CleanHeapSnapshotFiles(std::vector<uint64_t> ids);
```

`CleanHeapSnapshotFiles` 接受以下入参：

- ids：内存快照 ID 列表

清除对应的内存快照的解析数据。

##### 查询所有内存快照概览

```cpp
std::vector<HeapSnapshot> QueryAllHeapSnapshot();
```

`QueryAllHeapSnapshot` 返回所有内存快照的概览信息。

##### 获取内存快照 ID

```cpp
uint64_t GetSnapshotIDByFilePath(std::string filePath);
```

`GetSnapshotIDByFilePath` 接受以下入参：

- filePath：内存快照文件路径

返回对应的 ID。

##### 获取内存快照所有 Constructor 节点

```cpp
std::vector<ConstructorNode> GetConstructorNodesBySnapshotID(uint64_t id);
```

`GetConstructorNodesBySnapshotID` 接受以下入参：

- id：内存快照 ID

返回所有的 Constructor 节点。

##### 获取内存快照根节点

```cpp
std::vector<ConstructorNode> GetRootNodesBySnapshotID(uint64_t id, std::set<uint8_t> rootTypes);
```

`GetRootNodesBySnapshotID` 接受以下入参：

- id：内存快照 ID
- rootTypes：根节点类型集合

返回对应的 Constructor 节点列表。

`GetRootNodesBySnapshotID` 支持以下根节点类型：

- 0：非根节点
- 1：GLOBAL 根节点
- 2：LOCAL 根节点
- 3：UNKNOWN 根节点

##### 获取两个内存快照的差异根节点

```cpp
std::vector<ConstructorDiffNode> GetRootDiffNodesBySnapshotID(uint64_t baseSnapshotId, uint64_t targetSnapshotId, std::set<uint8_t> rootTypes);
```

`GetRootDiffNodesBySnapshotID` 接受以下入参：

- baseSnapshotId：基准内存快照 ID
- targetSnapshotId：目标内存快照 ID
- rootTypes：根节点类型集合

返回对应的差异 Constructor 节点列表。

`GetRootDiffNodesBySnapshotID` 支持以下根节点类型：

- 0：非根节点
- 1：GLOBAL 根节点
- 2：LOCAL 根节点
- 3：UNKNOWN 根节点

##### 展开 Constructor 节点

```cpp
ConstructorNode ExpandConstructorNode(uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);
```

`ExpandConstructorNode` 接受以下入参：

- snapshotId：内存快照 ID
- nodeId：待展开的 Constructor 节点 ID
- startIndex：分页开始位置，从 0 开始计数
- length：分页长度

返回展开后的 Constructor 节点。

##### 展开两个内存快照的差异 Constructor 节点

```cpp
ConstructorDiffNode ExpandConstructorDiffNode(uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);
```

`ExpandConstructorDiffNode` 接受以下入参：

- baseSnapshotId：基准内存快照 ID
- targetSnapshotId：目标内存快照 ID
- nodeId：待展开的 Constructor 节点 ID
- startIndex：分页开始位置，从 0 开始计数
- length：分页长度

返回展开后的差异 Constructor 节点。

##### 展开 Instance 节点

```cpp
InstanceNode ExpandInstanceNode(uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);
```

`ExpandInstanceNode` 接受以下入参：

- snapshotId：内存快照 ID
- nodeId：待展开的 Instance 节点 ID
- startIndex：分页开始位置，从 0 开始计数
- length：分页长度

返回展开后的 Instance 节点。

##### 展开两个内存快照的差异 Instance 节点

```cpp
InstanceDiffNode ExpandInstanceDiffNode(uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);
```

`ExpandInstanceDiffNode` 接受以下入参：

- baseSnapshotId：基准内存快照 ID
- targetSnapshotId：目标内存快照 ID
- nodeId：待展开的 Instance 节点 ID
- startIndex：分页开始位置，从 0 开始计数
- length：分页长度

返回展开后的差异 Instance 节点。

##### 展开 Instance 节点的属性节点或被引用节点

```cpp
InstanceNode ExpandDetailNode(uint64_t snapshotId, uint64_t nodeId, bool isReference, uint32_t startIndex, uint32_t length);
```

`ExpandDetailNode` 接受以下入参：

- snapshotId：内存快照 ID
- nodeId：待展开的 Instance 节点 ID
- isReference：`true` 表示展开属性节点，`false` 表示展开被引用节点
- startIndex：分页开始位置，从 0 开始计数
- length：分页长度

返回展开后的 Instance 节点。

##### 展开两个内存快照的差异 Instance 节点的属性节点或被引用节点

```cpp
InstanceDiffNode ExpandDetailDiffNode(uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, bool isReference, uint32_t startIndex, uint32_t length);
```

`ExpandDetailDiffNode` 接受以下入参：

- baseSnapshotId：基准内存快照 ID
- targetSnapshotId：目标内存快照 ID
- nodeId：待展开的 Instance 节点 ID
- isReference：`true` 表示展开属性节点，`false` 表示展开被引用节点
- startIndex：分页开始位置，从 0 开始计数
- length：分页长度

返回展开后的差异 Instance 节点。

##### 查看两个内存快照的差异信息

```cpp
std::vector<ConstructorDiffNode> QuerySnapshotComparison(uint64_t baseId, uint64_t targetId);
```

`QuerySnapshotComparison` 接受以下入参：

- baseId：基准内存快照 ID
- targetId：目标内存快照 ID

返回所有差异 Constructor 节点。

##### 获取某一 Instance 节点到根节点的路径

```cpp
std::vector<std::vector<InstanceNode>> GetNodeRootpaths(uint64_t snapshotId, uint64_t nodeId, int32_t pathNum);
```

`GetNodeRootpaths` 接受以下入参：

- snapshotId：内存快照 ID
- nodeId：待查询的 Instance 节点 ID
- pathNum：到根节点的路径数量，-1 表示全部数量

返回 Instance 节点到根节点的路径列表。

##### 获取线程信息

```cpp
std::vector<ThreadInfo> GetThreadInfos(uint64_t snapshotId);
```

`GetThreadInfos` 接受以下入参：

- snapshotId：内存快照 ID

返回内存快照中的所有线程信息。

##### 按关键字查询 Instance 节点的数量

```cpp
uint32_t QuerySnapshotCountOfResults(std::string keyword, bool isIgnoreCase, uint64_t snapshotId);
```

`QuerySnapshotCountOfResults` 接受以下入参：

- keyword：关键字
- isIgnoreCase：是否忽略大小写
- snapshotId：内存快照 ID

返回匹配的 Instance 节点的数量。

##### 查询按关键字搜索到的某一 Instance 节点所属的 Constructor 节点

```cpp
ConstructorNode QuerySnapshotNodeByIndex(std::string keyword, bool isIgnoreCase, uint64_t snapshotId, uint32_t length, uint32_t index);
```

`QuerySnapshotNodeByIndex` 接受以下入参：

- keyword：关键字
- isIgnoreCase：是否忽略大小写
- snapshotId：内存快照 ID
- length：Constructor 节点的子节点最大数量
- index：匹配的第 index 个 Instance 节点

返回匹配的 Instance 节点所属的 Constructor 节点。

##### 按关键字查询两个内存快照的差异 Instance 节点的数量

```cpp
uint32_t QueryComparisonCountOfResults(std::string keyword, bool isIgnoreCase, uint64_t baseId, uint64_t targetId);
```

`QueryComparisonCountOfResults` 接受以下入参：

- keyword：关键字
- isIgnoreCase：是否忽略大小写
- baseId：基准内存快照 ID
- targetId：目标内存快照 ID

返回匹配的差异 Instance 节点的数量。

##### 查询按关键字搜索到的某一差异 Instance 节点所属的差异 Constructor 节点

```cpp
ConstructorDiffNode QueryComparisonNodeByIndex(std::string keyword, bool isIgnoreCase, uint64_t baseId, uint64_t targetId, uint32_t length, uint32_t index);
```

`QueryComparisonNodeByIndex` 接受以下入参：

- keyword：关键字
- isIgnoreCase：是否忽略大小写
- baseId：基准内存快照 ID
- targetId：目标内存快照 ID
- length：Constructor 节点的子节点最大数量
- index：匹配的第 index 个 Instance 节点

返回匹配的差异 Instance 节点所属的差异 Constructor 节点。

##### class HeapSnapshot

`HeapSnapshot` 表示一个内存快照的概览信息，包含以下字段：

- uint64_t id：内存快照 ID
- uint64_t fileSize：内存快照文件大小
- std::string filePath：内存快照文件路径

##### class InstanceNode

`InstanceNode` 表示一个 Instance 节点的信息，包含以下字段：

- std::string className：类型名
- uint32_t distance：到根节点的最短距离
- uint32_t retainedSize：深堆大小
- uint32_t shallowSize：浅堆大小
- double shallowSizePercent：浅堆大小占总堆大小的百分比
- double retainedSizePercent：深堆大小占总堆大小的百分比
- uint32_t totalSize：总堆大小
- std::vector\<InstanceNode\> children：属性节点列表
- std::vector\<InstanceNode\> retainerNodes：被引用节点列表
- uint64_t id：节点 ID
- uint32_t nodeIndex：节点索引
- std::string type：节点类型
- std::string rootType：根节点类型
- uint32_t childrenCount：属性节点数量
- uint32_t retainerCount：被引用节点数量
- uint32_t startPosition：分页起始位置
- uint32_t endPosition：分页结束位置
- uint32_t arrayLength：数组中元素数量

##### class InstanceDiffNode

`InstanceDiffNode` 继承 `InstanceNode`，表示一个差异 Instance 节点的信息，新增以下字段：

- uint32_t addedCount：基准内存快照新增对象数量
- uint32_t removedCount：基准内存快照减少对象数量
- int64_t countDelta：新增对象数量与减少对象数量的差值
- uint32_t addedSize：基准内存快照新增对象的浅堆大小
- uint32_t removedSize：基准内存快照减少对象的浅堆大小
- int64_t sizeDelta：新增对象的浅堆大小与减少对象的浅堆大小的差值
- bool added：该 Instance 节点是否是新增对象

##### class ConstructorNode

`ConstructorNode` 表示一个 Constructor 节点的信息，包含以下字段：

- std::string className：类型名
- uint32_t totalSize：总堆大小
- uint64_t id：节点 ID
- uint32_t childrenCount：包含的 Instance 节点数量
- uint32_t distance：包含的 Instance 节点中到根节点的最短距离
- uint32_t shallowSize：浅堆大小
- uint32_t retainedSize：深堆大小
- double shallowSizePercent：浅堆大小占总堆大小的百分比
- double retainedSizePercent：深堆大小占总堆大小的百分比
- double totalInstanceCountPercent：包含的 Instance 节点数量占总 Instance 节点数量的百分比
- std::vector\<InstanceNode\> children：包含的 Instance 节点列表
- uint32_t startPosition：分页起始位置
- uint32_t endPosition：分页结束位置

##### class ConstructorDiffNode

`ConstructorDiffNode` 继承 `ConstructorNode`，表示一个差异 Constructor 节点的信息，新增以下字段：

- uint32_t addedCount：基准内存快照新增对象数量
- uint32_t removedCount：基准内存快照减少对象数量
- int64_t countDelta：新增对象数量与减少对象数量的差值
- uint32_t addedSize：基准内存快照新增对象的浅堆大小
- uint32_t removedSize：基准内存快照减少对象的浅堆大小
- int64_t sizeDelta：新增对象的浅堆大小与减少对象的浅堆大小的差值
- uint32_t baseTotalSize：基准内存快照总堆大小
- uint32_t targetTotalSize：目标内存快照总堆大小

##### class ThreadInfo

`ThreadInfo` 表示一个线程中的信息，包含以下字段：

- std::string name：线程名
- std::vector\<Frame\> frames：线程中的栈帧列表
- uint64_t id：线程 ID

其中 `Frame` 表示线程中一个栈帧信息，包含以下字段：

- std::string funcName：栈帧对应的函数名
- std::string fileName：栈帧对应的文件名
- int line：栈帧对应的源码行号
- std::vector\<InstanceNode\> locals：栈帧中的局部对象列表
- uint64_t id：栈帧 ID