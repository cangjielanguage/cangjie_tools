# HLE工具用户指南

## 功能简介

`HLE (HyperlangExtension)` 是一个仓颉调用ArkTS互操作代码模板自动生成工具。
该工具的输入是ArkTS接口声明文件，例如后缀.d.ts或者.d.ets结尾的文件，输出为包含BUILD.gn文件和src文件夹。src文件夹中包含的cj文件中存放生成的互操作代码。工具也会输出包含ArkTS文件的所有信息的json文件。

## 使用说明

### 参数含义

| 参数            | 含义                                        | 参数类型 | 说明                 |
| --------------- | ------------------------------------------- | -------- | -------------------- |
| `-i`            | d.ts或者d.ets文件输入的绝对路径             | 可选参数 | 和`-d`参数二选一或者两者同时存在                     |
| `-r`            | typescript编译器的绝对路径                  | 必选参数 |                      |
| `-d`            | d.ts或者d.ets文件输入所在文件夹的绝对路径    | 可选参数 | 和`-i`参数二选一或者两者同时存在                     |
| `-o`            | 输出保存互操作代码的目录                    | 可选参数 | 缺省时输出至当前目录 |
| `-j`            | 分析d.t或者d.ets文件的路径                 | 可选参数 |                      |
| `--module-name` | 自定义生成的仓颉包名                        | 可选参数 |                      |
| `--lib`         | 生成三方库代码                              | 可选参数 |                      |
| `--help`        | 帮助选项                                   | 可选参数 |                      |

### 命令行

可使用如下的命令生成接口胶水层代码：
```sh
main  -i  /path/to/test.d.ts  -o  out  –j  /path/to/analysis.js --module-name=ohos.hilog
```

在Windows环境，文件目录当前不支持符号“\\”，仅支持使用“/”。
```sh
main -i  /path/to/test.d.ts -o out -j /path/to/analysis.js --module-name=ohos.hilog
```
