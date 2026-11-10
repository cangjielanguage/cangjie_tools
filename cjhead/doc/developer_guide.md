# cjhead

## 开源项目介绍

`CJHead(Cangjie Head)`是仓颉 header file 生成工具：扫描仓颉源文件，生成对应头文件 cj.d。其整体技术架构如图所示：

![cjhead架构设计图](../figures/cjhead软件架构.png)

## 目录

` cjhead ` 源码目录如下图所示，其主要功能如注释中所描述。
```
cjhead/
|-- build                   # 构建脚本
|-- config                  # 配置文件
|-- doc                     # 介绍文档
|-- fmt                     # 格式化工具
|-- include                 # 头文件
│-- src                     # 源码
```

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

### Local Mode 适配

用户可见的导出规则见[用户指南](user_guide.md)。`--enable-local-mode` 只传给前端的 `globalOptions.enableLocalMode`，控制能否解析 local 语法，不影响声明过滤；构建时须使用支持 local mode 的配套编译器头文件和前端库。

声明过滤由 `HeadFileVisitor.cpp` 中的 `IsLocalOnlyDeclaration()` 统一判定，命中的声明从输出用的声明数组中删除，涵盖顶层声明、各类成员、enum 构造器及宏展开声明。判定只看声明签名，规则如下：

| 判定依据 | AST 位置 |
|---|---|
| 类型节点的模式为 `@ local!` / `@ local?`，即 `Type::modal.Local()` 为 `ASTMode::FULL` / `ASTMode::HALF` | 参数类型、返回类型 `FuncBody::retType`、变量和属性类型、类型别名目标、`InheritableDecl::inheritedTypes`、`ExtendDecl::extendedType`、`GenericConstraint::upperBounds`，并递归检查其中嵌套的类型节点（如函数类型的参数和返回类型） |
| 显式 `this` 的模式为 `@ local!` / `@ local?` | `FuncParamList::thisParam->modal` |
| `exclave` 修饰 | `FuncDecl::modifiers` 中的 `TokenKind::EXCLAVE` |
| 语义类型的模式为 `Mode::FULL` / `Mode::HALF` | `FuncDecl`、`VarDeclAbstract` 的 `GetTy()`，沿 `typeArgs` 递归；用于覆盖推断的返回类型和类型别名展开 |

以下内容不参与判定：函数体 `FuncBody::body`、名义类型所引用声明的成员、`Ref` 等引用指向的目标声明。前者避免误删只在实现中使用 local 的 API，后两者避免跨文件访问已被剪枝的声明。未写模式的类型（`ASTMode::NONE`，等价于 `@ ~local`）、显式写 `@ ~local` 的类型（`ASTMode::NOT`）及 `demode` 修饰符不触发过滤。

按模态类型的语言规则，模式只能出现在变量、函数参数和返回值、函数类型以及属性的类型中，其他需要类型的位置只能写数据类型。类型别名目标、enum 构造器参数、extend 的被扩展类型、实例成员变量、泛型实参等位置标注 `@ local!` / `@ local?` 会被编译器拒绝，全局变量和静态成员变量自身只能采用 `@ ~local`（函数类型内部的参数和返回类型除外），cjhead 无需单独处理。父类型和泛型约束上界中的模式目前仍能通过编译，因此保留对 `inheritedTypes` 和 `upperBounds` 的检查。

formatter 输出时不保留模式标注、显式 `this` 和 `exclave`，因此 local 专属声明必须删除，否则会以普通签名输出，并与同名的普通重载重复。

## 编译构建

### 基于源代码构建

下载并编译 cangjie 置于 third_party/cangjie 下

```bash
git clone <your cangjie repo> ./third_party/cangjie

cd ./third_party/cangjie

./build.py build --no-tests --cjnative -t release --enable-assert

./build.py install

source ./output/envsetup.sh

```

编译 cjhead

```bash
mkdir output && cd output && cmake .. && make
```

输出见 output 目录下的 bin

### 基于构建脚本构建

执行构建脚本

```bash
python build/build.py
```

完成一键构建，其中构建的配置文件位于 config/config.ini

配置文件的内容如下

```ini
[project]
# 构建环境的根目录
root = ./

[cangjie]
# repo of cangjie.
repo = <Your Cangjie repo>
# branch of cangjie repo.
branch = dev

[paths]
# 拷贝的 cangjie repo 存储路径
cangjie_dir = third_party/cangjie
# build 产物的存储路径
build_dir = build
cangjie_build_script = build.py
envsetup_sh = output/envsetup.sh
cjhead_src_dir = third_party

[build]
# 是否在构建时开启 test 模式
no_tests = true
# 是否在构建时开启 cjnative 模式
cjnative = true
# 是否在构建时开启 debug 模式
target = debug
# 是否在构建时开启 assert
enable_assert = true
```
