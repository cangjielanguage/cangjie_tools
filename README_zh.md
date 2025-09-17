# 仓颉语言命令行工具

## 简介

仓颉语言为开发者提供了丰富的命令行工具以及语言服务器工具，在成功安装仓颉工具链后，即可根据手册说明使用这些工具。

## 开源项目介绍

目前的仓颉工具如下：

- 仓颉包管理工具 `cjpm`
- 仓颉格式化工具 `cjfmt`
- 仓颉多语言桥接工具 `hle`
- 仓颉语言服务器 `lsp`

与命令行工具和语言服务器工具对应的软件架构图请参考：

- [`cjpm` 软件架构图](./cjpm/doc/developer_guide.md#开源项目介绍)
- [`cjfmt` 软件架构图](./cjfmt/doc/developer_guide.md#开源项目介绍)
- [`hle` 软件架构图](./hyperlangExtension/doc/developer_guide.md#开源项目介绍)
- [`lsp` 系统架构图](./cangjie-language-server/doc/developer_guide.md#开源项目介绍)

对应的目录结构分别为：

- [`cjpm` 目录](./cjpm/doc/developer_guide.md#目录)
- [`cjfmt` 目录](./cjfmt/doc/developer_guide.md#目录)
- [`hle` 目录](./hyperlangExtension/doc/developer_guide.md#目录)
- [`lsp` 目录](./cangjie-language-server/doc/developer_guide.md#目录)

工具的相关仓具体为：

- [`cjpm` 相关仓](./cjpm/doc/developer_guide.md#相关仓)
- [`cjfmt` 相关仓](./cjfmt/doc/developer_guide.md#相关仓)
- [`hle` 相关仓](./hyperlangExtension/doc/developer_guide.md#相关仓)
- [`lsp` 相关仓](./cangjie-language-server/doc/developer_guide.md#相关仓)

若想获取详细信息，请参阅各工具 `doc` 目录下的使用指南：

- `cjpm`:
    - [`cjpm` 用户指南](./cjpm/doc/user_guide.md)
    - [`cjpm` 开发者指南](./cjpm/doc/developer_guide.md)
- `cjfmt`:
    - [`cjfmt` 用户指南](./cjfmt/doc/user_guide.md)
    - [`cjfmt` 开发者指南](./cjfmt/doc/developer_guide.md)
- `hle`:
    - [`hle` 用户指南](./hyperlangExtension/doc/user_guide.md)
    - [`hle` 开发者指南](./hyperlangExtension/doc/developer_guide.md)
- `lsp`:
    - [`lsp` 用户指南](./cangjie-language-server/doc/user_guide.md)
    - [`lsp` 开发者指南](./cangjie-language-server/doc/developer_guide.md)

## 构建依赖

命令行工具构建依赖于仓颉 `SDK`，请参考[仓颉 SDK 集成构建指南](https://gitcode.com/Cangjie/cangjie_build/blob/dev/README_zh.md)

## 开源协议

本项目基于 [Apache-2.0 with Runtime Library Exception](./LICENSE)，请自由地享受和参与开源。

## 使用的开源软件声明

| 开源软件名称               | 开源许可协议              | 使用说明                  | 使用主体 | 使用方式         |
|----------------------|---------------------|-----------------------|------|--------------|
| flatbuffers          | Apache License V2.0 | 仓颉语言服务对索引数据进行序列化和反序列化 | 语言服务 | 集成到仓颉二进制发布包中 |
| JSON  for Modern C++ | MIT License         | 仓颉语言服务用于报文解析和封装       | 语言服务 | 集成到仓颉二进制发布包中 |
| SQLite               | Public Domain       | 仓颉语言服务使用数据库存储索引数据     | 语言服务 | 集成到仓颉二进制发布包中 |

## 参与贡献

欢迎开发者们提供任何形式的贡献，包括但不限于代码、文档、issue 等。
