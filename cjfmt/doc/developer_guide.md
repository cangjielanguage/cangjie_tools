# Cangjie Formatter Developer Guide

## System Architecture

`cjfmt (Cangjie Formatter)` is a code formatting tool specifically designed for the Cangjie language. `cjfmt` supports automatically adjusting code indentation, spacing, and line breaks to help developers maintain clean and consistent code style effortlessly. Its overall technical architecture is shown in the following diagram:

![cjfmt Architecture Diagram](../figures/cjfmt-architecture.jpg)

As illustrated in the architecture diagram, the overall architecture of `cjfmt` is as follows:

- Command-line Parameter Management: The command parameter processing module of `cjfmt` supports file-level source code formatting, directory-level source code formatting, and block-level code formatting via commands. It also handles formatting style configuration and formatting result output.

- Configuration Management: Users configure formatting styles via the `cangjie-format.toml` configuration file, including indentation width, line width limits, and line break styles.

- Input Module: Processes input for formatting source code, accepting source code directories, files, or snippets, and forwards them to the formatting module for processing.

- Source Code Compilation: Invokes Cangjie's frontend capabilities to perform lexical and syntactic analysis on source code awaiting formatting, constructing the corresponding Abstract Syntax Tree (AST).

- Formatting: Traverses AST nodes to generate a nested intermediate structure. Applies formatting strategies to process this intermediate structure and transforms it into the target formatted source code.

- Output Module: Processes the formatted source code output, supporting overwriting the input file or outputting to a new directory or file.

## Directory Structure

The source code directory of `cjfmt` is shown below, with main functionalities described in the comments.
```
cjfmt/
|-- build                   # Build scripts
|-- config                  # Configuration files
|-- doc                     # Documentation
|-- include                 # Header files
|-- src
    |-- Format
        |-- DocProcessor    # Converts Doc struct to source code
        |-- NodeFormatter    # Converts AST nodes to Doc struct
```

## Installation and Usage Guide

`cjfmt` requires the following tools for building:

- `clang` or `gcc` compiler

### Build Preparation

`cjfmt` depends on `cjc` for building. Refer to [SDK Build]() for build instructions.

### Build Steps

Local build process:

1. Get the latest source code via `git clone`:

    ```shell
    cd ${WORKDIR}
    git clone https://gitcode.com/Cangjie/cangjie_tools.git
    ```

2. Configure environment variables:

    ```shell
    export CANGJIE_HOME=${WORKDIR}/cangjie    (for Linux/macOS)
    set CANGJIE_HOME=${WORKDIR}/cangjie       (for Windows)
    ```

    `cjfmt` compilation depends on `cangjie` build artifacts, so the `CANGJIE_HOME` environment variable must point to the SDK location. `${WORKDIR}/cangjie` is just an example - adjust according to actual SDK location.

   > **Note:**
   >
   > - On Windows, ensure correct directory separators are used and Chinese characters in paths are properly handled.

3. Compile `cjfmt` using build scripts in `cjfmt/build`:

    ```shell
    cd cangjie_tools/cjfmt/build
    python3 build.py build -t release
    ```

    Currently supports `debug` and `release` build types, specified via `-t` or `--build-type`.

4. Install to target directory:

    ```shell
    python3 build.py install
    ```

    Default installation path is `cjfmt/dist`. Developers can specify installation directory via `--prefix`:

    ```shell
    python3 build.py install --prefix ./output
    ```

    Build output structure:

    ```
    dist/
    |-- bin
        `-- cjfmt                   # Executable (cjfmt.exe on Windows)
    |-- config
        `-- cangjie-format.toml     # Formatter config file
    ```

5. Verify installation:

    ```shell
    ./cjfmt -h
    ```

    Execute this in the `bin` directory. If help info is displayed, installation succeeded. Note: The `cjfmt` executable depends on `cangjie-lsp` dynamic library - ensure library path is in system environment variables. For Linux:

    ```shell
    export LD_LIBRARY_PATH=$CANGJIE_HOME/tools/lib:$LD_LIBRARY_PATH
    ./cjfmt -h
    ```

6. Clean build artifacts:

   ```shell
   python3 build.py clean
   ```

Cross-compiling for Windows from Linux:

```shell
export CANGJIE_HOME=${WORKDIR}/cangjie
python3 build.py build -t release --target windows-x86_64
python3 build.py install
```

Output will be in `cjfmt/dist`. Note: Windows version SDK is required for cross-compilation.

### Additional Build Options

View all build parameters via:

```shell
python3 build.py build -h
```

## API and Configuration Reference

`cjfmt` provides the following main commands for project building and configuration management.

### Command Overview

Usage: `cjfmt [option] file [option] file`

`cjfmt -h` displays help info and options:

```text
Usage:
     cjfmt -f fileName [-o fileName] [-l start:end]
     cjfmt -d fileDir [-o fileDir]
Options:
   -h            Show usage
                     eg: cjfmt -h
   -v            Show version
                     eg: cjfmt -v
   -f            Specifies the file in the required format. The value can be a relative path or an absolute path.
                     eg: cjfmt -f test.cj
   -d            Specifies the file directory in the required format. The value can be a relative path or an absolute path.
                     eg: cjfmt -d test/
   -o <value>    Output. If a single file is formatted, '-o' is followed by the file name. Relative and absolute paths are supported;
                 If a file in the file directory is formatted, a path must be added after -o. The path can be a relative path or an absolute path.
                     eg: cjfmt -f a.cj -o ./fmta.cj
                     eg: cjfmt -d ~/testsrc -o ./testout
   -c <value>    Specify the format configuration file, Relative and absolute paths are supported.
                 If the specified configuration file fails to be read, cjfmt will try to read the default configuration file in CANGJIE_HOME
                 If the default configuration file also fails to be read, will use the built-in configuration.
                     eg: cjfmt -f a.cj -c ./config/cangjie-format.toml
                     eg: cjfmt -d ~/testsrc -c ~/home/project/config/cangjie-format.toml
   -l <region>   Only format lines in the specified region for the provided file. Only valid if a single file was specified.
                 Region has a format of [start:end] where 'start' and 'end' are integer numbers representing first and last lines to be formated in the specified file.
                 Line count starts with 1.
                     eg: cjfmt -f a.cj -o ./fmta.cj -l 1:25
```

### File Formatting

`cjfmt -f`

- Format and overwrite source file (supports relative/absolute paths):

```shell
cjfmt -f ../../../test/uilang/Thread.cj
```

- Use `-o` to output formatted code to new file:

```shell
cjfmt -f ../../../test/uilang/Thread.cj -o ../../../test/formated/Thread.cj
```

### Directory Formatting

`cjfmt -d`

- Format all Cangjie source files in specified directory:

```shell
cjfmt -d test/              // Relative path

cjfmt -d /home/xxx/test     // Absolute path
```

- Use `-o` to specify output directory (will be created if nonexistent). Note OS-specific path length limits (e.g. 260 chars on Windows, 4096 on Linux):

```shell
cjfmt -d test/ -o /home/xxx/testout

cjfmt -d /home/xxx/test -o ../testout/

cjfmt -d testsrc/ -o /home/../testout   // Error if source directory doesn't exist
```

### Configuration File

`cjfmt -c`

- Specify custom formatting configuration file:

```shell
cjfmt -f a.cj -c ./cangjie-format.toml
```

### Partial Formatting

`cjfmt -l`

- Format only specified line range (works only with `-f`):

```shell
cjfmt -f a.cj -o .cj -l 10:25 // Formats only lines 10-25
```

## Related Repositories

- [cangjie repo](https://gitcode.com/Cangjie/cangjie_compiler)
- [SDK Build](https://gitcode.com/Cangjie/cangjie_build)