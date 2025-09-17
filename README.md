# Cangjie Language Command Line Tools

## Introduction

The Cangjie language provides a rich set of command line tools and language server tool for developers. After successfully installing the Cangjie toolchain, you can use these tools according to the manual instructions.

## Open Source Project Introduction

The current Cangjie tools include:

- Cangjie Package Manager `cjpm`
- Cangjie Formatter `cjfmt`
- Cangjie HyperLang Extension `hle`
- Cangjie Languager Server `lsp`

Please refer to the following software architecture diagrams for the command-line tools and language server tools:

- [software architecture diagram for `cjpm`](./cjpm/doc/developer_guide.md#开源项目介绍)
- [software architecture diagram for `cjfmt`](./cjfmt/doc/developer_guide.md#开源项目介绍)
- [software architecture diagram for `hle`](./hyperlangExtension/doc/developer_guide.md#开源项目介绍)
- [software architecture diagram for `lsp`](./cangjie-language-server/doc/developer_guide.md#开源项目介绍)

The corresponding directory structures are as follows:

- [the directory of `cjpm`](./cjpm/doc/developer_guide.md#目录)
- [the directory of `cjfmt`](./cjfmt/doc/developer_guide.md#目录)
- [the directory of `hle`](./hyperlangExtension/doc/developer_guide.md#目录)
- [the directory of `lsp`](./cangjie-language-server/doc/developer_guide.md#目录)

The corresponding associated repositories for the tools are as follows:

- [the associated repositories of `cjpm`](./cjpm/doc/developer_guide.md#相关仓)
- [the associated repositories of `cjfmt`](./cjfmt/doc/developer_guide.md#相关仓)
- [the associated repositories of `hle`](./hyperlangExtension/doc/developer_guide.md#相关仓)
- [the associated repositories of `lsp`](./cangjie-language-server/doc/developer_guide.md#相关仓)

To get detailed information, please refer to the user guides in the corresponding doc directory:

- `cjpm`:
    - [User Guide for `cjpm`](./cjpm/doc/user_guide.md)
    - [Developer Guide for `cjpm`](./cjpm/doc/developer_guide.md)
- `cjfmt`:
    - [User Guide for `cjfmt`](./cjfmt/doc/user_guide.md)
    - [Developer Guide for `cjfmt`](./cjfmt/doc/developer_guide.md)
- `hle`:
    - [User Guide for `hle`](./hyperlangExtension/doc/user_guide.md)
    - [Developer Guide for `hle`](./hyperlangExtension/doc/developer_guide.md)
- `lsp`:
    - [User Guide for `lsp`](./cangjie-language-server/doc/user_guide.md)
    - [Developer Guide for `lsp`](./cangjie-language-server/doc/developer_guide.md)

## Construction Dependencies

The construction of tools relies on Cangjie `SDK`. Please refer to [Cangjie SDK Integration Construction Guide](https://gitcode.com/Cangjie/cangjie_build/blob/dev/README.md)

## Open Source License

This project is licensed under [Apache-2.0 with Runtime Library Exception](./LICENSE). Please enjoy and participate in open source freely.

## Open Source Software Statement

| Software Name        | License             | Usage Description                                               | Main Component | Usage Method                               |
|----------------------|---------------------|-----------------------------------------------------------------|----------------|--------------------------------------------|
| flatbuffers          | Apache License V2.0 | Cangjie Language Server serializes and deserializes index data. | LSPServer      | Integrated into the Cangjie binary release |
| JSON  for Modern C++ | MIT License         | Cangjie Language Server for message parsing and encapsulation.  | LSPServer      | Integrated into the Cangjie binary release |
| SQLite               | Public Domain       | Cangjie Language Server uses the database to store index data.  | LSPServer      | Integrated into the Cangjie binary release |

## Contribution

We welcome contributions from developers in any form, including but not limited to code, documentation, issues, and more.