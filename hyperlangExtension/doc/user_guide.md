# HLE Tool User Guide

## Introduction

`HLE (HyperlangExtension)` is a tool for automatically generating interoperability code templates for Cangjie calling ArkTS or C language.
The input of this tool is the interface declaration file of ArkTS or C language, such as files ending with .d.ts, .d.ets or .h, and the output is a cj file, which stores the generated interoperability code. If the generated code is a glue layer code from ArkTS to Cangjie, the tool will also output a json file containing all the information of the ArkTS file.

## Instructions

### Parameter Meaning

| Parameter           | Meaning                                       | Parameter Type | Description                  |
| ------------------- | --------------------------------------------- | -------------- | ---------------------------- |
| `-i`                | Absolute path of d.ts, d.ets or .h file input | Optional       | Choose one from `-d` or both |
| `-r`                | Absolute path of typescript compiler          | Required       |                              |
| `-d`                | Absolute path of the folder where d.ts, d.ets or .h file input is located | Optional       | Choose one from `-i` or both |
| `-o`                | Directory to save the output interoperability code | Optional       | Output to the current directory by default |
| `-j`                | Path to analyze d.t or d.ets files            | Optional       |                              |
| `--module-name`     | Custom generated Cangjie package name         | Optional       |                              |
| `--lib`             | Generate third-party library code             | Optional       |                              |
| `-c`                | Generate C to Cangjie binding code             | Optional       |                              |
| `-b`                | Specify the path of the cjbind binary             | Optional       |                              |
| `--clang-args`      | Parameters that will be directly passed to clang | Optional       |                              |
| `--no-detect-include-path` | Disable automatic include path detection    | Optional       |                              |
| `--help`            | Help option                                   | Optional       |                              |

### Command Line

You can use the following command to generate ArkTS to Cangjie binding code:

```sh
main  -i  /path/to/test.d.ts  -o  out  –j  /path/to/analysis.js --module-name=ohos.hilog
```

In the Windows environment, the file directory currently does not support the symbol "\\", only "/" is supported.
```sh
main -i  /path/to/test.d.ts -o out -j /path/to/analysis.js --module-name=ohos.hilog
```

You can use the following command to generate C to Cangjie binding code:

```sh
./target/bin/main -c --module-name="my_module" -d ./tests/c_cases -o ./tests/expected/c_module/ --clang-args="-I/usr/lib/llvm-20/lib/clang/20/include/"
```

The `-b` parameter is used to specify the path of the cjbind binary, with the default value being "./src/dtsparser/node_modules/.bin/cjbind". During [Build Preparation](./developer_guide.md#build-preparation), the cjbind binary will be downloaded to the "./src/dtsparser/node_modules/.bin/" directory. You can specify a different cjbind path by using the `-b` parameter. Here is an example command:

```sh
./target/bin/main -b ./src/dtsparser/node_modules/.bin/cjbind -c --module-name="my_module" -d ./tests/c_cases -o ./tests/expected/c_module/ --clang-args="-I/usr/lib/llvm-20/lib/clang/20/include/"
```