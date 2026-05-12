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
