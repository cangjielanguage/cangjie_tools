# 仓颉兼容检查工具

## 功能简介

`cjcompat (Cangjie Compatible)` 仓颉兼容检查工具是一款基于仓颉语言 `Library Evolution` 规范开发的 `ABI` / `API` 兼容检查工具。

## 使用说明

使用命令行操作 `cjcompat --old cjofile --new cjofile` 。

`cjcompat -h` 帮助信息，选项介绍。

```text
Usage:
       cjcompat --old /path/to/old.cjo --new /path/to/new.cjo [--api] [--abi] [--import-path /path]
Options:
   -h                      Show usage
                               eg: cjcompat -h
   --old <value>           Path to the .cjo file corresponding to the old version of a Cangjie library.
   --new <value>           Path to the .cjo file corresponding to the new version of a Cangjie library.
                               eg: cjcompat --old filePathOld --new filePathNew
   --import-path <value>   Add .cjo search path.
   --api                   Enforces rules for API compatibility.
   --abi                   Enforces rules for ABI compatibility.
                               eg: cjcompat --old filePathOld --new filePathNew --api
```

### 兼容规则检查

`cjcompat --old cjofile --new cjofile`

- 检查不同版本的 `cjo` 是否兼容，`cjofile` 支持相对路径和绝对路径。

```shell
cjcompat --old old.cjo --new new.cjo
```

> **说明：**
>
> 规则检查时必须指定 `--old` 和 `--new` 选项且仅允许出现一次，否则报错。
> 不同版本 `cjofile` 必须以 `.cjo` 结尾，否则报错。
> 不同版本 `cjofile` 必须是兼容版本内 `cjc` 编译生成的，否则可能因版本不兼容而出现 `cjo` 文件解析错误。
> 不指定检查规则类型时，默认 `api` 和 `abi` 规则都检查。
> 如果出现 `Cannot load file old.cjo` 报错提示，则可能还依赖其他 `cjo` 文件，可以通过使用 `CANGJIE_PATH` 环境变量或 `--import-path` 来指定依赖的 `cjo` 文件路径。

### api 兼容规则检查

`cjcompat --api`

- 选项 `--api` 让用户指定仅检查 `api` 兼容规则。

```shell
cjcompat --old old.cjo --new new.cjo --api
```

### abi 兼容规则检查

`cjcompat --abi`

- 选项 `--abi` 让用户指定仅检查 `abi` 兼容规则。

```shell
cjcompat --old old.cjo --new new.cjo --abi
```

## 兼容性规则

- 具体兼容性规则请参考仓颉模块演进兼容性规范 `(library evolution spec)` 。

其中，全局变量变更中新增和删除场景的兼容性规则如下：

| 变更行为 | `API` 兼容性 | `ABI` 兼容性 |
|-|-|-|
| 新增一个全局变量 | 兼容 | 兼容 |
| 删除一个不带 `public` 修饰符的全局变量 | 兼容 | 兼容 |
| 删除一个带 `public` 修饰符的全局变量 | 不兼容 | 不兼容 |

按照上述兼容性规则举例如下：

【正例】

```cangjie
// old.cj
package A

public var x: Int64 = 10

//==================================//

// new.cj
package A

public var x: Int64 = 10
var y: Int64 = 10 // 新增一个全局变量
```

使用该工具对上述仓颉源码生成的 `cjo` 文件进行规则检查。

```shell
cjcompat --old old.cjo --new new.cjo
```

运行结果如下：

```text
Compatible!
```

> **说明：**
>
> 新增一个全局变量，检查结果为 `API` 和 `ABI` 都兼容。

【正例】

```cangjie
// old.cj
package A

public var x: Int64 = 10
var y: Int64 = 10 // 删除一个不带 public 修饰符的全局变量

//==================================//

// new.cj
package A

public var x: Int64 = 10
```

使用该工具对上述仓颉源码生成的 `cjo` 文件进行规则检查。

```shell
cjcompat --old old.cjo --new new.cjo
```

运行结果如下：

```text
Compatible!
```

> **说明：**
>
> 删除一个不带 `public` 修饰符的全局变量，检查结果为 `API` 和 `ABI` 都兼容。

【反例】

```cangjie
// old.cj
package A

public var x: Int64 = 10
public var y: Int64 = 10 // 删除一个带 public 修饰符的全局变量

//==================================//

// new.cj
package A

public var x: Int64 = 10
```

使用该工具对上述仓颉源码生成的 `cjo` 文件进行规则检查。

```shell
cjcompat --old old.cjo --new new.cjo
```

运行结果如下：

```text
Incompatible!

[API/ABI] Variable declaration visible outside the module deleted or made private.
  var_decl y: [BEFORE] A/new.cj:4:1-4:25
```

> **说明：**
>
> 删除一个带 `public` 修饰符的全局变量，检查结果为 `API` 和 `ABI` 都不兼容。

【反例】

```cangjie
// old.cj
package A

public var x: Int64 = 10
public var y: Int64 = 10 // 删除一个带 public 修饰符的全局变量

//==================================//

// new.cj
package A

public var x: Int64 = 10
```

使用该工具对上述仓颉源码生成的 `cjo` 文件进行 `api` 规则检查。

```shell
cjcompat --old old.cjo --new new.cjo --api
```

运行结果如下：

```text
Incompatible!

[API] Variable declaration visible outside the module deleted or made private.
  var_decl y: [BEFORE] A/new.cj:4:1-4:25
```

> **说明：**
>
> 用户指定仅检查 `api` 规则时，删除一个带 `public` 修饰符的全局变量，检查结果为 `API` 不兼容。

【反例】

```cangjie
// old.cj
package A

public var x: Int64 = 10
public var y: Int64 = 10 // 删除一个带 public 修饰符的全局变量

//==================================//

// new.cj
package A

public var x: Int64 = 10
```

使用该工具对上述仓颉源码生成的 `cjo` 文件进行 `abi` 规则检查。

```shell
cjcompat --old old.cjo --new new.cjo --abi
```

运行结果如下：

```text
Incompatible!

[ABI] Variable declaration visible outside the module deleted or made private.
  var_decl y: [BEFORE] A/new.cj:4:1-4:25
```

> **说明：**
>
> 用户指定仅检查 `abi` 规则时，删除一个带 `public` 修饰符的全局变量，检查结果为 `ABI` 不兼容。
