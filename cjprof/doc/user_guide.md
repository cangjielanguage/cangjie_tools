# Cangjie Profile Tool

## Overview

`cjprof (Cangjie Profile)` is a performance profiling tool for the Cangjie programming language. It supports the following features:

- Perform CPU hot function sampling on Cangjie programs and export sampling data.
- Analyze hot function sampling data and generate CPU hot function statistical reports or flame graphs.
- Dump heap memory of Cangjie applications and analyze it to generate analysis reports.

Currently, `cjprof` supports the above features on `Linux` systems, and only supports heap memory analysis for Cangjie applications on `macOS` and `Windows` systems.

## Usage Instructions

Run `cjprof --help` to view command usage. It supports the `record`, `report`, and `heap` subcommands, which are used to collect CPU hot function information, generate CPU hot function reports (including flame graphs), and dump and analyze heap memory respectively.

```text
cjprof --help
 Usage: cjprof [--help] COMMAND [ARGS]

The supported commands are:
  -v        Print version of cjprof
  heap      Dump heap into a dump file or analyze the heap dump file
  record    Run a command and record its profile data into data file
  report    Read profile data file (created by cjprof record) and display the profile
```

> **Note:**
>
> Since `cjprof record` relies on the system's `perf` permissions, one of the following two conditions must be met for usage:
> - Execute with the `root` user or `sudo` privileges.
> - Set the system's `perf_event_paranoid` parameter (via the `/proc/sys/kernel/perf_event_paranoid` file) to -1.
>
> Otherwise, insufficient permission errors may occur.

### Collecting CPU Hot Function Information

#### Command

```text
cjprof record
```

#### Syntax

```text
cjprof record [<options>] [<command>]
cjprof record [<options>] -- <command> [<options>]
```

#### Options

`-f, --freq <freq>`
Specifies the sampling frequency in hertz (Hz), i.e., the number of samples per second. The default value is 5000 Hz. If set to `max` or a value exceeding the maximum frequency supported by the system, the maximum supported frequency will be used.

`-o, --output <file>`
Specifies the filename of the sampling data generated after sampling. The default is `cjprof.data`.

`-p, --pid <pid>`
Specifies the process ID of the application to be sampled. This option is ignored when a new application is launched and sampled via `<command>`.

#### Examples

- Sample a running application.

```text
# Sample the running application (PID 12345) at a frequency of 10000 Hz, and generate sampling data in the current directory named sample.data after sampling completes.
cjprof record -f 10000 -p 12345 -o sample.data
```

- Launch a new application and sample it.

```text
# Execute the `test` application in the current directory with arguments `arg1 arg2`, sample it at the maximum frequency supported by the system, and generate sampling data in the current directory named `cjprof.data` (default filename) after sampling completes.
cjprof record -f max -- ./test arg1 arg2
```

#### Notes

- After sampling starts, it will only end when the sampled program exits. To stop sampling early, press `Ctrl+C` during the sampling process.

### Generating CPU Hot Function Reports

#### Command

```text
cjprof report
```

#### Syntax

```text
cjprof report [<options>]
```

#### Options

`-F, --flame-graph`
Generates a CPU hot function flame graph instead of the default text report.

`-i, --input <file>`
Specifies the sampling data file. The default is `cjprof.data`.

`-o, --output <file>`
Specifies the filename of the generated CPU hot function flame graph. The default is `FlameGraph.svg`. This option only takes effect when generating a flame graph.

#### Examples

- Generate the default CPU hot function text report.

```text
# Analyze sampling data in sample.data and generate a CPU hot function text report.
cjprof report -i sample.data
```

- Generate a CPU hot function flame graph.

```text
# Analyze sampling data in cjprof.data (default file) and generate a CPU hot function flame graph named test.svg.
cjprof report -F -o test.svg
```

#### Report Format Description

- The text report includes three parts: total sampling percentage of the function (including child functions), self sampling percentage of the function, and function name (displayed as an address if no corresponding symbol information is available). Results are sorted in descending order by total sampling percentage.

- In the flame graph, the horizontal axis represents the sampling percentage — wider bars indicate a higher sampling percentage and longer execution time. The vertical axis represents the call stack, with parent functions below and child functions above.

### Dumping and Analyzing Heap Memory

#### Command

```text
cjprof heap
```

#### Syntax

```text
cjprof heap [<options>]
```

#### Options

`-D, --depth <depth>`
Specifies the maximum display depth of object reference/referenced relationships. The default is 10 levels. This option only takes effect when `--show-reference` is specified.

`-d, --dump <pid>`
Dumps the current heap memory of a Cangjie application, where `pid` is the process ID of the application. It also works if a child thread ID of the application is provided.

`-i, --input <file>`
Specifies the heap memory data file to analyze. The default is `cjprof.data`.

`-o, --output <file>`
Specifies the filename of the exported heap memory data. The default is `cjprof.data`.

`--show-reference[=<objnames>]`
Displays object reference relationships in the analysis report. `objnames` are the names of objects to display, separated by `;` for multiple objects. When not specified, all objects are displayed by default. If the object is a root node of the heap, the root node category of the heap it belongs to will also be displayed.

`--incoming-reference`
Displays referenced-by relationships of objects instead of reference relationships. Must be used together with `--show-reference`.

`-t, --show-thread`
Displays Cangjie thread stacks and objects referenced on the stack in the analysis report.

`-V, --verbose`
Diagnostic option. Prints parsing logs when parsing heap memory data files.

#### Examples

- Export heap memory data.

```text
# Dump the current heap memory of the running application (PID 12345) to a file named heap.data in the current directory.
cjprof heap -d 12345 -o heap.data
```

> **Note:**
>
> Dumping heap memory sends a `SIG_USR1` signal to the target process. Exercise caution when unsure whether the target process is a Cangjie application, as sending the signal incorrectly may cause unexpected errors.
> Both the directory of the running Cangjie program and the directory where the dump command is executed require write permissions; otherwise, the operation may fail due to insufficient permissions.

- Analyze heap memory data and display object information.

```text
# Parse and analyze the heap memory data file heap.data in the home directory, displaying the object type name, instance count, shallow heap size, and retained heap size of each live object in the heap.
cjprof heap -i ~/heap.data
```

The output of the above command is as follows:

```text
Object Type           Objects        Shallow Heap   Retained Heap
====================  =============  =============  =============
AAA                               1            80             400
BBB                               4            32             196
CCC                               2            16              32
```

- Analyze heap memory data and display Cangjie thread stacks and object references.

```text
# Parse and analyze the heap memory data file cjprof.data (default file) in the current directory, displaying Cangjie thread stacks and objects referenced on the stack.
cjprof heap --show-thread
```

The output of the above command is as follows:

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

- Analyze heap memory data and display object reference relationships.

```text
# Parse and analyze the heap memory data file cjprof.data (default file) in the current directory, displaying reference relationships of objects of type AAA and BBB.
cjprof heap --show-reference="AAA;BBB"
```

The output of the above command is as follows:

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

- Analyze heap memory data and display referenced-by relationships of objects.

```text
# Parse and analyze the heap memory data file cjprof.data (default file) in the current directory, displaying referenced-by relationships of objects of type CCC.
cjprof heap --show-reference="CCC" --incoming-reference
```

The output of the above command is as follows:

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

#### Heap Memory Analysis Report Explanation

- Object type names `RawArray<Byte>[]`, `RawArray<Half>[]`, `RawArray<Word>[]`, and `RawArray<DWord>[]` represent primitive raw arrays of 1-byte, 2-byte, 4-byte, and 8-byte sizes respectively.

- **Shallow Heap** refers to the heap memory size occupied by the object itself.

- **Retained Heap** refers to the sum of the shallow heap sizes of all objects that can be freed (i.e., objects directly or indirectly referenced by the object) after the object is garbage collected.

- When the object reference hierarchy exceeds the maximum display depth, or duplicate objects appear due to circular references, `...` is used to omit subsequent references.

#### Memory Analysis API

The `cjprof` dynamic library provides the following memory analysis APIs to support secondary development.

##### Parse Heap Snapshot Files

```cpp
bool ParseHeapSnapshotFiles(std::vector<std::string>& filePaths);
```

`ParseHeapSnapshotFiles` accepts the following input parameters:

- filePaths: List of heap snapshot file paths

Returns `true` only when all heap snapshots are parsed successfully.

##### Clean Heap Snapshot Data

```cpp
void CleanHeapSnapshotFiles(std::vector<uint64_t> ids);
```

`CleanHeapSnapshotFiles` accepts the following input parameters:

- ids: List of heap snapshot IDs

Clears the parsed data of corresponding heap snapshots.

##### Query Overview of All Heap Snapshots

```cpp
std::vector<HeapSnapshot> QueryAllHeapSnapshot();
```

`QueryAllHeapSnapshot` returns overview information of all heap snapshots.

##### Get Heap Snapshot ID

```cpp
uint64_t GetSnapshotIDByFilePath(std::string filePath);
```

`GetSnapshotIDByFilePath` accepts the following input parameters:

- filePath: Heap snapshot file path

Returns the corresponding ID.

##### Get All Constructor Nodes of Heap Snapshot

```cpp
std::vector<ConstructorNode> GetConstructorNodesBySnapshotID(uint64_t id);
```

`GetConstructorNodesBySnapshotID` accepts the following input parameters:

- id: Heap snapshot ID

Returns all Constructor nodes.

##### Get Root Nodes of Heap Snapshot

```cpp
std::vector<ConstructorNode> GetRootNodesBySnapshotID(uint64_t id, std::set<uint8_t> rootTypes);
```

`GetRootNodesBySnapshotID` accepts the following input parameters:

- id: Heap snapshot ID
- rootTypes: Collection of root node types

Returns the corresponding list of Constructor node.

`GetRootNodesBySnapshotID` supports the following root node types:

- 0: Non-root node
- 1: GLOBAL root node
- 2: LOCAL root node
- 3: UNKNOWN root node

##### Get Diff Root Nodes of Two Heap Snapshots

```cpp
std::vector<ConstructorDiffNode> GetRootDiffNodesBySnapshotID(uint64_t baseSnapshotId, uint64_t targetSnapshotId, std::set<uint8_t> rootTypes);
```

`GetRootDiffNodesBySnapshotID` accepts the following input parameters:

- baseSnapshotId: Base heap snapshot ID
- targetSnapshotId: Target heap snapshot ID
- rootTypes: Collection of root node types

Returns the corresponding list of diff Constructor nodes.

`GetRootDiffNodesBySnapshotID` supports the following root node types:

- 0: Non-root node
- 1: GLOBAL root node
- 2: LOCAL root node
- 3: UNKNOWN root node

##### Expand Constructor Node

```cpp
ConstructorNode ExpandConstructorNode(uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);
```

`ExpandConstructorNode` accepts the following input parameters:

- snapshotId: Heap snapshot ID
- nodeId: ID of the Constructor node to be expanded
- startIndex: Pagination start position, counting from 0
- length: Pagination length

Returns the expanded Constructor node.

##### Expand Diff Constructor Node of Two Heap Snapshots

```cpp
ConstructorDiffNode ExpandConstructorDiffNode(uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);
```

`ExpandConstructorDiffNode` accepts the following input parameters:

- baseSnapshotId: Base heap snapshot ID
- targetSnapshotId: Target heap snapshot ID
- nodeId: ID of the Constructor node to be expanded
- startIndex: Pagination start position, counting from 0
- length: Pagination length

Returns the expanded diff Constructor node.

##### Expand Instance Node

```cpp
InstanceNode ExpandInstanceNode(uint64_t snapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);
```

`ExpandInstanceNode` accepts the following input parameters:

- snapshotId: Heap snapshot ID
- nodeId: ID of Instance node to be expanded
- startIndex: Pagination start position, counting from 0
- length: Pagination length

Returns the expanded Instance node.

##### Expand Diff Instance Node of Two Heap Snapshots

```cpp
InstanceDiffNode ExpandInstanceDiffNode(uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, uint32_t startIndex, uint32_t length);
```

`ExpandInstanceDiffNode` accepts the following input parameters:

- baseSnapshotId: Base heap snapshot ID
- targetSnapshotId: Target heap snapshot ID
- nodeId: ID of the diff Instance node to be expanded
- startIndex: Pagination start position, counting from 0
- length: Pagination length

Returns the expanded diff Instance node.

##### Expand Attribute Nodes or Referenced Nodes of Instance Node

```cpp
InstanceNode ExpandDetailNode(uint64_t snapshotId, uint64_t nodeId, bool isReference, uint32_t startIndex, uint32_t length);
```

`ExpandDetailNode` accepts the following input parameters:

- snapshotId: Heap snapshot ID
- nodeId: ID of the Instance node to be expanded
- isReference: `true` for expanding attribute nodes, `false` for expanding referenced nodes
- startIndex: Pagination start position, counting from 0
- length: Pagination length

Returns the expanded Instance node.

##### Expand Attribute Nodes or Referenced Nodes of Diff Instance Node Between Two Heap Snapshots

```cpp
InstanceDiffNode ExpandDetailDiffNode(uint64_t baseSnapshotId, uint64_t targetSnapshotId, uint64_t nodeId, bool isReference, uint32_t startIndex, uint32_t length);
```

`ExpandDetailDiffNode` accepts the following input parameters:

- baseSnapshotId: Base heap snapshot ID
- targetSnapshotId: Target heap snapshot ID
- nodeId: ID of the Instance node to be expanded
- isReference: `true` for expanding attribute nodes, `false` for expanding referenced nodes
- startIndex: Pagination start position, counting from 0
- length: Pagination length

Returns the expanded diff Instance node.

##### Query Diff Information Between Two Heap Snapshots

```cpp
std::vector<ConstructorDiffNode> QuerySnapshotComparison(uint64_t baseId, uint64_t targetId);
```

`QuerySnapshotComparison` accepts the following input parameters:

- baseId: Base heap snapshot ID
- targetId: Target heap snapshot ID

Returns all diff Constructor nodes.

##### Get Paths from An Instance Node to Root Node

```cpp
std::vector<std::vector<InstanceNode>> GetNodeRootpaths(uint64_t snapshotId, uint64_t nodeId, int32_t pathNum);
```

`GetNodeRootpaths` accepts the following input parameters:

- snapshotId: Heap snapshot ID
- nodeId: ID of the Instance node to query
- pathNum: Number of paths to the root node, -1 means all paths

Returns the list of paths from the Instance node to the root node.

##### Get Thread Information

```cpp
std::vector<ThreadInfo> GetThreadInfos(uint64_t snapshotId);
```

`GetThreadInfos` accepts the following input parameters:

- snapshotId: Heap snapshot ID

Returns all thread information in the heap snapshot.

##### Query Instance Node Count by Keyword

```cpp
uint32_t QuerySnapshotCountOfResults(std::string keyword, bool isIgnoreCase, uint64_t snapshotId);
```

`QuerySnapshotCountOfResults` accepts the following input parameters:

- keyword: Search keyword
- isIgnoreCase: Whether to ignore case
- snapshotId: Heap snapshot ID

Returns the count of matched Instance nodes.

##### Query Constructor Node of A Specific Instance Node Searched by Keyword

```cpp
ConstructorNode QuerySnapshotNodeByIndex(std::string keyword, bool isIgnoreCase, uint64_t snapshotId, uint32_t length, uint32_t index);
```

`QuerySnapshotNodeByIndex` accepts the following input parameters:

- keyword: Search keyword
- isIgnoreCase: Whether to ignore case
- snapshotId: Heap snapshot ID
- length: Maximum number of child nodes of the Constructor node
- index: The Nth matched Instance node

Returns the Constructor node that the matched Instance node belongs to.

##### Query Diff Instance Node Count of Two Heap Snapshots by Keyword

```cpp
uint32_t QueryComparisonCountOfResults(std::string keyword, bool isIgnoreCase, uint64_t baseId, uint64_t targetId);
```

`QueryComparisonCountOfResults` accepts the following input parameters:

- keyword: Search keyword
- isIgnoreCase: Whether to ignore case
- baseId: Base heap snapshot ID
- targetId: Target heap snapshot ID

Returns the count of matched diff Instance nodes.

##### Query Diff Constructor Node of A Specific Diff Instance Node Searched by Keyword

```cpp
ConstructorDiffNode QueryComparisonNodeByIndex(std::string keyword, bool isIgnoreCase, uint64_t baseId, uint64_t targetId, uint32_t length, uint32_t index);
```

`QueryComparisonNodeByIndex` accepts the following input parameters:

- keyword: Search keyword
- isIgnoreCase: Whether to ignore case
- baseId: Base heap snapshot ID
- targetId: Target heap snapshot ID
- length: Maximum number of child nodes of the Constructor node
- index: The Nth matched diff Instance node

Returns the diff Constructor node that the matched diff Instance node belongs to.

##### class HeapSnapshot

`HeapSnapshot` represents overview information of a heap snapshot, containing the following fields:

- uint64_t id: Heap snapshot ID
- uint64_t fileSize: Heap snapshot file size
- std::string filePath: Heap snapshot file path

##### class InstanceNode

`InstanceNode` represents information of an Instance node, containing the following fields:

- std::string className: Type name
- uint32_t distance: Shortest distance to the root node
- uint32_t retainedSize: Retained heap size
- uint32_t shallowSize: Shallow heap size
- double shallowSizePercent: Percentage of shallow heap size in total heap size
- double retainedSizePercent: Percentage of retained heap size in total heap size
- uint32_t totalSize: Total heap size
- std::vector\<InstanceNode\> children: List of attribute nodes
- std::vector\<InstanceNode\> retainerNodes: List of referenced nodes
- uint64_t id: Node ID
- uint32_t nodeIndex: Node index
- std::string type: Node type
- std::string rootType: Root node type
- uint32_t childrenCount: Number of attribute nodes
- uint32_t retainerCount: Number of referenced nodes
- uint32_t startPosition: Pagination start position
- uint32_t endPosition: Pagination end position
- uint32_t arrayLength: Number of elements in the array

##### class InstanceDiffNode

`InstanceDiffNode` inherits from `InstanceNode` and represents information of a diff Instance node, with the following additional fields:

- uint32_t addedCount: Number of objects added in the base heap snapshot
- uint32_t removedCount: Number of objects removed in the base heap snapshot
- int64_t countDelta: Difference between added object count and removed object count
- uint32_t addedSize: Shallow heap size of objects added in the base heap snapshot
- uint32_t removedSize: Shallow heap size of objects removed in the base heap snapshot
- int64_t sizeDelta: Difference between shallow heap size of added objects and removed objects
- bool added: Whether the Instance node is a newly added object

##### class ConstructorNode

`ConstructorNode` represents information of a Constructor node, containing the following fields:

- std::string className: Type name
- uint32_t totalSize: Total heap size
- uint64_t id: Node ID
- uint32_t childrenCount: Number of contained Instance nodes
- uint32_t distance: Shortest distance to the root node among contained Instance nodes
- uint32_t shallowSize: Shallow heap size
- uint32_t retainedSize: Retained heap size
- double shallowSizePercent: Percentage of shallow heap size in total heap size
- double retainedSizePercent: Percentage of retained heap size in total heap size
- double totalInstanceCountPercent: Percentage of contained Instance node count in total Instance node count
- std::vector\<InstanceNode\> children: List of contained Instance nodes
- uint32_t startPosition: Pagination start position
- uint32_t endPosition: Pagination end position

##### class ConstructorDiffNode

`ConstructorDiffNode` inherits from `ConstructorNode` and represents information of a diff Constructor node, with the following additional fields:

- uint32_t addedCount: Number of objects added in the base heap snapshot
- uint32_t removedCount: Number of objects removed in the base heap snapshot
- int64_t countDelta: Difference between added object count and removed object count
- uint32_t addedSize: Shallow heap size of objects added in the base heap snapshot
- uint32_t removedSize: Shallow heap size of objects removed in the base heap snapshot
- int64_t sizeDelta: Difference between shallow heap size of added objects and removed objects
- uint32_t baseTotalSize: Total heap size of base heap snapshot
- uint32_t targetTotalSize: Total heap size of target heap snapshot

##### class ThreadInfo

`ThreadInfo` represents thread information, containing the following fields:

- std::string name: Thread name
- std::vector\<Frame\> frames: List of stack frames in the thread
- uint64_t id: Thread ID

`Frame` represents a stack frame information in the thread, containing the following fields:

- std::string funcName: Function name corresponding to the stack frame
- std::string fileName: Source file name corresponding to the stack frame
- int line: Source code line number corresponding to the stack frame
- std::vector\<InstanceNode\> locals: List of local objects in the stack frame
- uint64_t id: Stack frame ID