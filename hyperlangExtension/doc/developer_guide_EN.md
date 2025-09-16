# HLE Tool Development Guide

## Open Source Project Introduction

`HLE (HyperlangExtension)` is an automated code template generation tool for Cangjie-ArkTS interoperability.
The tool takes ArkTS interface declaration files (e.g., files with `.d.ts` or `.d.ets` extensions) as input and outputs a directory containing `BUILD.gn` files and an `src` folder. The `src` folder contains generated interoperability code in `.cj` files. The tool also outputs a JSON file containing all ArkTS file information. The overall technical architecture is shown below:

![HLE Architecture Diagram]()

## Directory Structure

The source code directory of `HLE` is structured as follows, with main functionalities described in the comments:
```
|-- build   # Build scripts
|-- doc     # Documentation
|-- src     # Source code
    |-- dtsparser # ArkTS interface file parser
    |-- entry # ArkTS-to-Cangjie interface converter
    |-- tool # Utility classes for the conversion process
```

## Installation and Usage Guide:

### Build Preparation

Below is a build guide for Ubuntu 22 environment.

1. Download and extract the latest Cangjie package, then configure the Cangjie environment:

`HLE` requires the following tools for building:

- `Cangjie SDK`
    - Developers need to download the `Cangjie SDK` for the target platform: For compiling native platform artifacts, use the SDK version matching the current platform; for cross-compiling Windows artifacts from Linux, use the Linux version of the SDK.
    - Then, execute the `envsetup` script of the corresponding SDK to ensure proper configuration.
- `stdx` binary library compatible with the `Cangjie SDK`
    - Download the `stdx` binary library for the target platform. For cross-compiling Windows artifacts from Linux, use the Windows version of `stdx`.
    - Configure the `stdx` binary library path to the environment variable `CANGJIE_STDX_PATH`, pointing to the `static/stdx` directory under the extracted library.
    - Alternatively, to use `stdx` dynamic libraries as dependencies, replace `static` with `dynamic` in the path configuration. Note that `HLE` compiled this way cannot run independently unless the same `stdx` library path is added to the system's dynamic library environment variables.
- For building with `python` scripts, install `python3`.

2. This tool requires Node.js for execution:

    Recommended version: v18.14.1 or higher. Lower versions may fail to parse certain ArkTS syntax, so using the latest version is advised.

    [How to Install Node.js](https://dev.nodejs.cn/learn/how-to-install-nodejs/)

    For example, use the following commands:

    ```sh
    # Download and install nvm:
    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash
    # in lieu of restarting the shell
    \. "$HOME/.nvm/nvm.sh"
    # Download and install Node.js:
    nvm install 22
    # Verify the Node.js version:
    node -v # Should print "v22.17.1".
    nvm current # Should print "v22.17.1".
    # Verify npm version:
    npm -v # Should print "10.9.2".
    ```

### Build Steps

1. Clone this project:

    ```sh
    git clone https://gitcode.com/Cangjie/cangjie_tools.git
    ```

2. Install dependencies:

    Install TypeScript dependencies, which include the TypeScript compiler, by running:

    ```sh
    cd {WORKDIR}/cangjie-tools/hyperlangExtension/src/dtsparser
    npm install
    ```

3. Compile `hle` using the build script in `hyperlangExtension/build`:

    ```shell
    cd cangjie-tools/hyperlangExtension/build
    python3 build.py build -t release
    ```

    Currently supported build types are `debug` and `release`, specified via `-t` or `--build-type`.

4. Install to a specified directory:

    ```shell
    python3 build.py install
    ```

    By default, this installs to `hyperlangExtension/target`. Use the `--prefix` parameter to specify an alternative directory:

    ```shell
    python3 build.py install --prefix ./output
    ```

    The output directory structure is:

    ```
    output/
    |-- hle                         # Executable (hle.exe on Windows)
    ```

5. Verify `hle` installation:

    ```shell
    ./hle -h
    ```

    Execute this in the installation directory. If the help information for `hle` is displayed, the installation is successful.

    To use the binary file directly:

    ```sh
    ${WORKDIR}/cangjie-tools/hyperlangExtension/output/hle -i input_file_path -o output_folder --lib
    ```

    Example:

    ```sh
    ${WORKDIR}/cangjie-tools/hyperlangExtension/output/hle  -i  ${WORKDIR}/cangjie-tools/hyperlangExtension/tests/cases/class.d.ts -o out --module-name=ohos.hilog --lib
    ```

    On Windows, file paths must use forward slashes (`/`) instead of backslashes (`\`):
    ```sh
    ${WORKDIR}/cangjie-tools/hyperlangExtension/output/hle.exe -i  ${WORKDIR}/cangjie-tools/hyperlangExtension/tests/cases/class.d.ts -o out --module-name=ohos.hilog --lib
    ```

6. Clean build artifacts:

   ```shell
   python3 build.py clean
   ```

Cross-compiling `hle` for Windows from Linux is also supported:

```shell
export CANGJIE_HOME=${WORKDIR}/cangjie
python3 build.py build --target windows-x86_64
python3 build.py install --prefix output
```

The build artifacts will be located in `hyperlangExtension/output`.

### Additional Build Options

`build.py` provides the following additional options for the `build` command:
- `--target TARGET`: Specify the target platform (default: `native`). Currently, only cross-compiling `windows-x86_64` artifacts from `linux` is supported via `--target windows-x86_64`;
- `-t, --build-type BUILD_TYPE`: Specify build type (`debug` or `release`);
- `-h, --help`: Display help information for the `build` command.

Other `build.py` functionalities include:

- `install [--prefix PREFIX]`: Install build artifacts to a specified path (default: `hyperlangExtension/target/`). Requires successful `build` execution first;
- `clean`: Remove build artifacts from the default path;
- `-h, --help`: Display general help information for `build.py`.

### Command Reference

Usage: `hle [option] file [option] file`

`hle -h` displays help information and options:

```text
Usage: main [options]

Description:
    This tool is designed to generate cangjie bindings for .d.ts or .d.ets files.

Options:
  -i <file>             The absolute path of the input d.ts or d.ets file (required if -d is not used)
  -r <file>             The absolute path of the typescript compiler source code
  -d <directory>        The absolute path of the directory containing d.ts or d.ets file (required if -i is not used)
  -o <directory>        The directory to save the binding code (optional, defaults to the current directory)
  -j <file>             The absolute path of the d.ts or d.ets file analyzer (optional)
  --module-name <name>  Customize the generated Cangjie package name (optional)
  --lib                 Generate bindings for third-party library (optional)
  --help                Display this help information
```

### Test Case Validation

Execute test cases with the following command. Output will be saved in `./tests/expected/my_module/`, and users should verify if it meets expectations.

```bash
${WORKDIR}/cangjie-tools/hyperlangExtension/output/hle--lib --module-name="my_module" -d ./tests/cases -o ./tests/expected/my_module/
```

## Related Repositories

- [Cangjie Repository](https://gitcode.com/Cangjie/cangjie_compiler)
- [stdx Repository](https://gitcode.com/Cangjie/cangjie_stdx)
- [Cangjie SDK](https://gitcode.com/Cangjie/cangjie_build)