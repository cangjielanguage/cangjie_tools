# cjhead

仓颉 header file 生成工具：扫描仓颉文件，生成对应头文件

## 导出规则

### Package

- 无修饰符的 package：默认 public  
- public 修饰的 package  
- protected package 里面按下述规则对外可见的 API，且该 API 在其他 public 包内被 public import  

### Import

- 无修饰符的 import  
- public import  
- protected/internal/private import  

### Macro

- public 修饰的全局 macro  

### Class / Struct / Enum / Interface

- public/sealed 修饰的 class  
- public 修饰的 struct  
- public 修饰的 enum  
- public/sealed 修饰的 interface  

### Variable

- public 修饰的全局变量  
- public class/struct 中 public 修饰的成员变量  
- sealed class 中 public 修饰的成员变量  
- public(但非 sealed) open/abstract class 的 protected 成员变量：因为在子类中可见  

### Type Alias

- public 修饰的全局类型别名  

### Function

- public 修饰的全局函数  
- public/sealed 修饰的 class/struct/interface/enum 中的 public 修饰的成员函数  
- public/sealed interface 中无修饰符的成员函数（interface中无修饰符=public）  
- public(但非 sealed) open/abstract class 的 protected 修饰的成员函数：因为在子类中可见  

### Property

- public/sealed class/struct/interface/enum 中的 public 修饰的成员 prop  
- public/sealed interface 中无修饰符成员 prop（interface中无修饰符=public）  
- public(但非sealed) open/abstract class 的 protect 修饰的成员 prop：因为在子类中可见  

### Extend

- extend 的 public 成员，extend 与被扩展类型在同一个包（无论是否是接口扩展），被扩展类型是 public 的，扩展的泛型约束里用到的类型是 public 的  
- extend 的 public 成员，extend 是接口扩展，接口与被扩展的类型不在同一个包，被扩展类型是 public 的，接口是 public 的，扩展的泛型约束中用到的类型也是 public 的，且该成员继承自被扩展的 public 接口  
- extend 的 protected 成员，extend 与被扩展类型在同一个包（无论是否是接口扩展），被扩展类型是 public(但非 sealed) open/abstract class，扩展的泛型约束里用到的类型是public的  

### Comment & Annotation

- 单行注释、多行注释  
- 可见声明对应的注解  

### Macro Expansion

- 宏展开的函数  

### 处理含 local 语法的源码

处理含 local 语法的源码时，使用配套编译器版本构建的 cjhead，并显式传入 `--enable-local-mode`：

```bash
cjhead -i <Input Dir> -o <Output Dir> --enable-local-mode
```

local mode 目前是实验特性，编译器前端默认不识别 local 相关语法。只有开启此选项，cjhead 才能解析 `@ local!`、`this @ local?`、`exclave` 等写法，从而识别出需要过滤的 local 专属 API。未开启时，前端会报告 `local mode is disabled`，这些声明无法被正确识别，生成的头文件可能有误，例如推断出 local 返回类型的函数被导出、`demode` 成员变量缺失。

此选项默认关闭，适用于单文件、目录、`-p` 合包及宏展开。开启后 `exclave`、`demode` 成为关键字，因此仅在源码使用 local 语法时开启。此选项不会使 local 专属 API 被导出。

#### 不导出的 API

只要 API 的对外签名需要使用方开启 local 模式才能书写或调用，该 API 就不导出，即使它满足上文的可见性规则。具体包括：

| 情形 | 示例 |
|---|---|
| 参数类型或返回类型带 `@ local!` / `@ local?` 的函数、成员函数、构造函数，包括函数类型参数中的标注 | `public func total(xs: Array<Int64> @ local?): Int64`<br>`public func each(f: (String @ local?) -> Unit): Unit` |
| 显式声明 `this @ local!` / `this @ local?` 的成员函数、构造函数 | `public func size(this @ local?): Int64`<br>`public exclave init(this @ local!, item: String @ local!)` |
| 类型带 `@ local!` / `@ local?` 的实例成员属性 | `public prop last: Option<T> @ local!` |
| `exclave` 修饰的函数、构造函数 | `public exclave func redirect(paths: Array<String> @ local!, target: String @ local!): Unit` |
| 类型为带上述标注的函数类型的全局变量、常量 | `public let handler: (String @ local?) -> Unit = ...` |
| 未写返回类型、但推断出的返回类型带 `@ local!` / `@ local?` 的函数 | `public func makeSquares(n: Int64) { exclave { Array<Int64>(n, {i => i * i}) @ local! } }` |

以下 API 按原有规则导出：

- 只在函数体内使用 local 的 API，例如函数体内创建 `Array<Int64>(n, {i => i}) @ local!`。头文件不包含函数体。
- 与不导出的 API 同名的普通重载。例如同时存在 `func f(x: String)` 和 `func f(x: String @ local?)` 时，前者照常导出。
- 显式写 `@ ~local` 的 API：`@ ~local` 与不写模式等价，生成的头文件中省略该标注。
- `demode` 修饰的成员变量：生成的头文件中省略 `demode` 修饰符。

## 参数说明

```bash
$ ./bin/cjhead -h
Usage:  cjhead [OPTION...]

OPTION:
  -i,              Input directory
  -o,              Output directory(default: .\)
  -p,              Enable package mode, save all content in one package to one file
  --import-path,   Cjo path to scan
  --macro-path,    Macro path to resolve the macro
  --enable-local-mode, Accept local syntax in input; local-only APIs are not exported
  --exclude,       Config file describe files not to scan
  -h,              Print help
```

运行 cjhead:

```bash
cjhead -i <Input Dir> -o <Output Dir> -p --import-path <Your CJO Path> --macro-path <Your Macro path>
```

其中 `-p` 参数表示开启 package 模式，此时所有位于相同包下的 cj 文件会被整合到一个 cjd 文件中，`--import-path` 参数指定 cjo 文件的位置，能够帮助 cjhead 正确识别 cj 文件的依赖信息，`--macro-path` 参数指定宏定义文件的位置，能够帮助 cjhead 正确地识别并解析宏

### 示例1

输入：

```cangjie
/**
 * Rectangle
 */
public class Rectangle {
    // width
    let width: Int64
    let height: Int64

    /**
     * Rectangle::init
     */
    public init(width: Int64, height: Int64) {
        this.width = width
        this.height = height
    }
    /**
     * Rectangle::area
     */
    public func area() {
        width * height
    }
}
```

输出：

```cangjie
/**
 * Rectangle
 */
public class Rectangle {
    /**
     * Rectangle::init
     */
    public init(width: Int64, height: Int64)

    /**
     * Rectangle::area
     */
    public func area()
}
```

### 示例2

输入：

```cangjie
/**
 * The Digest interface
 */
public interface Digest {
    prop size: Int64

    prop blockSize: Int64

    prop algorithm: String

    func write(buffer: Array<Byte>): Unit

    func finish(to!: Array<Byte>): Unit

    func finish(): Array<Byte> {
        let buf = Array<Byte>(size, repeat: 0)
        this.finish(to: buf)
        return buf
    }

    func reset(): Unit
}

/**
 * The function is to calculate the digest of data
 *
 * @param algorithm - digest type
 * @param data - data to be digested
 *
 * @return message-digested data
 */
@Deprecated[message: "Use global function `public func digest<T>(algorithm: T, input: InputStream): Array<Byte> where T <: Digest` instead."]
public func digest<T>(algorithm: T, data: String): Array<Byte> where T <: Digest {
    digest(algorithm, data.toArray())
}
```

输出：

```cangjie
/**
 * The Digest interface
 */
public interface Digest {
    prop size: Int64

    prop blockSize: Int64

    prop algorithm: String

    func write(buffer: Array<Byte>): Unit

    func finish(to!: Array<Byte>): Unit

    func finish(): Array<Byte>

    func reset(): Unit
}

/**
 * The function is to calculate the digest of data
 *
 * @param algorithm - digest type
 * @param data - data to be digested
 *
 * @return message-digested data
 */
@Deprecated[
    message: "Use global function `public func digest<T>(algorithm: T, input: InputStream): Array<Byte> where T <: Digest` instead."
]
public func digest<T>(algorithm: T, data: String): Array<Byte> where T <: Digest

```
