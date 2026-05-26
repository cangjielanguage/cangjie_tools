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
