# cangjie_tools

仓颉语言为开发者提供了丰富的命令行工具以及语言服务工具，在成功安装仓颉工具链后，即可根据手册说明使用这些工具。

## 目录结构

```
/cangjie_tools
├─ cangjie-language-server/  # 仓颉语言服务工具
├─ cjfmt/                    # 仓颉格式化工具
├─ cjpm/                     # 仓颉包管理工具
└─ hyperlangExtension/       # ArkTS 互操作代码模板自动生成工具
```

## OpenHarmony 如何使用 cangjie_tools

OpenHarmony 编译使用 `cangjie_tools` 各目录下的 `src` 源码文件，和部分目录下的 `include` 头文件。`cangjie_tools` 作为 OpenHarmony 的依赖模块，提供各项工具类功能，包括：

- 包管理功能；
- 格式化功能；
- `ArkTS` 互操作代码模板自动生成功能；
- 仓颉语言服务功能。

## OpenHarmony 如何集成 cangjie_tools

OpenHarmony 通过 `cangjie_tools` 各目录下的 `build` 目录内的提供的构建脚本 `build.py`，将源码文件编译成可执行文件。请参阅各工具对应的开发者指南，获取各工具构建脚本的使用方式：

- [`cjpm` 开发者指南](./cjpm/doc/developer_guide.md)
- [`cjfmt` 开发者指南](./cjfmt/doc/developer_guide.md)
- [`hle` 开发者指南](./hyperlangExtension/doc/developer_guide.md)
- [`lsp` 开发者指南](./cangjie-language-server/doc/developer_guide.md)

各工具的编译产物均为可执行文件，可以直接使用。请参阅各工具对应的用户指南，获取各工具可执行程序的使用方式：

- [`cjpm` 用户指南](./cjpm/doc/user_guide.md)
- [`cjfmt` 用户指南](./cjfmt/doc/user_guide.md)
- [`hle` 用户指南](./hyperlangExtension/doc/user_guide.md)
- [`lsp` 用户指南](./cangjie-language-server/doc/user_guide.md)

## License

基于 [Apache-2.0 with Runtime Library Exception](./LICENSE) 协议。

## 使用的开源软件声明

| 开源软件名称               | 开源许可协议              | 使用说明                  | 使用主体 | 使用方式         |
|----------------------|---------------------|-----------------------|------|--------------|
| flatbuffers          | Apache License V2.0 | 仓颉语言服务对索引数据进行序列化和反序列化 | 语言服务 | 集成到仓颉二进制发布包中 |
| JSON for Modern C++  | MIT License         | 仓颉语言服务用于报文解析和封装       | 语言服务 | 集成到仓颉二进制发布包中 |
| SQLite               | Public Domain       | 仓颉语言服务使用数据库存储索引数据     | 语言服务 | 集成到仓颉二进制发布包中 |

## 风险提示

**cangjie_tools 是 Apache-2.0 with Runtime Library Exception 协议类型的三方开源软件，使用时需履行相应的开源义务。**