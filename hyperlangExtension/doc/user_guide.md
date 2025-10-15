# HLE Tool User Guide

## Overview

`HLE (HyperlangExtension)` is an automated code template generation tool for Cangjie-to-ArkTS interoperability calls.
The tool takes ArkTS interface declaration files (e.g., files with `.d.ts` or `.d.ets` extensions) as input and outputs a directory containing a `BUILD.gn` file and an `src` folder. The `src` folder contains generated interoperability code in `.cj` files. The tool also outputs a JSON file containing all information from the ArkTS files.

## Usage Instructions

### Parameter Definitions

| Parameter       | Description                                      | Type       | Notes                |
| --------------- | ------------------------------------------------ | ---------- | -------------------- |
| `-i`            | Absolute path to input `.d.ts` or `.d.ets` file  | Optional   | Mutually exclusive or combinable with `-d`         |
| `-r`            | Absolute path to TypeScript compiler             | Required   |                      |
| `-d`            | Absolute path to directory containing input `.d.ts` or `.d.ets` files | Optional   | Mutually exclusive or combinable with `-i`         |
| `-o`            | Output directory for interoperability code       | Optional   | Defaults to current directory |
| `-j`            | Path to analyze `.d.ts` or `.d.ets` files       | Optional   |                      |
| `--module-name` | Custom Cangjie package name                      | Optional   |                      |
| `--lib`         | Generate third-party library code               | Optional   |                      |
| `--help`        | Help option                                      | Optional   |                      |

### Command Line

Use the following command to generate interface glue layer code:
```sh
main -i /path/to/test.d.ts -o out -j /path/to/analysis.js --module-name=ohos.hilog
```

On Windows environments, file paths do not support the "\\" symbol—only "/" is supported:
```sh
main -i /path/to/test.d.ts -o out -j /path/to/analysis.js --module-name=ohos.hilog
```
