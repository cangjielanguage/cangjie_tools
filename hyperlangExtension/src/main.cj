// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle

import std.argopt.*
import std.fs.*
import std.collection.*
import std.collection.concurrent.*
import std.env.*
import std.time.*
import std.sync.*
import stdx.serialization.serialization.*
import stdx.encoding.json.*
import std.process.*
import hle.tool.*
import hle.entry.*

const OS_WINDOWS = "Windows"
const OS_OTHERS = "Others"
@When[os == "Windows"]
const OS = OS_WINDOWS
@When[os != "Windows"]
const OS = OS_OTHERS
public const analysisFilePath: String = "./src/dtsparser/analysis.js"
let mtx = Mutex()
let cla = CommandLineArg()
let HELP_INFO = """
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

Examples:
  main -i /path/to/input.d.ts -o /path/to/output -j /path/to/analysis.js
        Process the specified d.ts file, use the d.ts file analyzer, and output to the specified directory
  main -d /path/to/input/folder -o /path/to/output -j /path/to/analysis.js
        Process the d.ts or d.ets file in the specified folder, use the d.ts file analyzer, and output to the specified directory
  main -d /path/to/input/folder -o /path/to/output -j /path/to/analysis.js --module-name=ohos.hilog
        Process the d.ts or d.ets file in the specified folder, use the d.ts file analyzer, customize the package name, and output to the specified directory
"""

func printHelp(): Unit {
    println(HELP_INFO)
}

main(args: Array<String>) {
    let start = MonoTime.now()

    let argSpecs = [
        Long("module-name", RequiredValue),
        Long("lib", NoValue),
        Long("help", NoValue),
        Short(r'r', RequiredValue),
        Short(r'i', RequiredValue),
        Short(r'o', RequiredValue),
        Short(r'j', RequiredValue),
        Short(r'd', RequiredValue),
        Short(r'h', NoValue)
    ]
    let pa = parseArguments(args, argSpecs)
    if (pa.options.contains("help") || pa.options.contains("h")) {
        printHelp()
        return 0
    }
    if (!pa.nonOptions.isEmpty()) {
        println("Error: Unkonwn option: '${pa.nonOptions}'\n")
        return 1
    }
    parseCommandLine(pa)

    // we want to get `lz4js` from `src/dtsparser/node_modules/@types/lz4js/`
    var moduleNameTemp = cla.cjModuleName
    cla.directoryPath = (pa.options.get("d") ?? "")
    if (!cla.directoryPath.isEmpty() && moduleNameTemp == "") {
        if (!cla.directoryPath.endsWith("/")) {
            cla.directoryPath += "/"
        }
        let dir = cla.directoryPath.split("/")
        if (dir.size >= 2) {
            moduleNameTemp = dir[dir.size - 2]
        }
    }

    // Parse one file
    if (!cla.arkpath.isEmpty()) {
        if (cla.arkpath.contains("/") && moduleNameTemp == "") {
            let dir = cla.arkpath.split("/")
            if (dir.size >= 2) {
                moduleNameTemp = dir[dir.size - 2]
            }
        }
        parseOneFiles(moduleNameTemp)
    }

    // Parse multiple files under a directory
    if (!cla.directoryPath.isEmpty() && !parseMultipleFiles(moduleNameTemp)) {
        return 1
    }

    let end = MonoTime.now()
    println("total cost of hle: ${end - start}")
    return 0
}

func parseCommandLine(pa: ParsedArguments): Unit {
    let currentPath = getHomeDirectory().toString()

    cla.arkpath = (pa.options.get("i") ?? "") //  输入的ArkTS文件路径
    cla.outDir = (pa.options.get("o") ?? currentPath) //  输出的Cangjie文件路径
    if (!cla.outDir.endsWith("/")) {
        cla.outDir += "/"
    }
    cla.jsASTPath = (pa.options.get("j") ?? analysisFilePath) //  ArkTS文件分析器路径
    cla.cjModuleName = (pa.options.get("module-name") ?? "")
    cla.is3rd = pa.options.contains("lib")

    @When[cjc_version < "0.59.4"]
    let pathName = pa.options.get("r") ?? "typescript"

    @When[cjc_version >= "0.59.4"]
    let pathName = Path(pa.options.get("r") ?? "typescript").normalize().toString()

    cla.tsPath = pathName
}

func parseMultipleFiles(moduleNameTemp: String): Bool {
    let fileInfos = ArrayList<FileInfo>()
    let dir = Path(cla.directoryPath)
    getFiles(fileInfos, dir)
    if (fileInfos.isEmpty()) {
        println("Error: generation failed! Not found .d.ts or .d.ets file under ${cla.directoryPath}")
        return false
    }

    let futureList = ArrayList<Future<Unit>>()
    let gconfigList = ConcurrentLinkedQueue<GlobalConfig>()
    for (f in fileInfos) {
        let future = spawn {
            parseFilesSpawn(f, dir, moduleNameTemp, gconfigList)
        }
        futureList.add(future)
    }
    for (f in futureList) {
        f.get()
    }

    // collect parent properties and methods in the whole module
    let gconfigArray = ArrayList<GlobalConfig>()
    for (_ in 0..gconfigList.size) {
        let gconfig = gconfigList.remove().getOrThrow()
        gconfigArray.add(gconfig)
    }

    for (gconfig in gconfigArray) {
        gconfig.gconfigArray = gconfigArray
        gconfig.collectParentPropertiesAndMethods()
    }

    // generate cj files
    futureList.clear()
    for (gconfig in gconfigArray) {
        let future = spawn {
            let a2cj = A2CJ(gconfig)
            a2cj.transfer()
        }
        futureList.add(future)
    }
    for (f in futureList) {
        f.get()
    }

    return true
}

func parseOneFiles(moduleNameTemp: String): Unit {
    let gconfig: GlobalConfig = GlobalConfig(cla.cjModuleName, cla.is3rd)
    parseFile(cla.arkpath, cla.outDir, cla.jsASTPath, gconfig, moduleNameTemp: moduleNameTemp,
        tsPath: cla.tsPath)
    gconfig.gconfigArray = ArrayList<GlobalConfig>([gconfig])
    gconfig.collectParentPropertiesAndMethods()
    let a2cj = A2CJ(gconfig)
    a2cj.transfer()
}

func parseFilesSpawn(f: FileInfo, dir: Path, name: String, gconfigList: ConcurrentLinkedQueue<GlobalConfig>): Unit {
    let basePath = dir.toString().replace("\\", "/")
    let moduleName = f.path.toString().replace("\\", "/")

    // Get each file's relative path to input direcotry
    var relativeFilePath = moduleName.split(basePath)[1]
    if (relativeFilePath != "" && Rune(relativeFilePath[0]) == r'/') {
        relativeFilePath = relativeFilePath[1..relativeFilePath.size - 1]
    }

    let relativePathName = Path(relativeFilePath).parent.toString()

    let gconfig: GlobalConfig = GlobalConfig(cla.cjModuleName, cla.is3rd,
        typeDeclFileArg: getTypeDeclFile(f))
    parseFile(moduleName, cla.outDir, cla.jsASTPath, gconfig, moduleNameTemp: name,
        thirdpartyName: moduleName.split("/")[moduleName.split("/").size - 1], relativePathName: relativePathName,
        tsPath: cla.tsPath)
    let _ = gconfigList.add(gconfig)
}

func getTypeDeclFile(file: FileInfo): String {
    let packageJsonFile = "oh-package.json5"
    if (let Some(dir) <- file.parentDirectory) {
        if (File.exists(dir.path.toString() + "/" + packageJsonFile)) {
            let fileContent = File.readFrom(dir.path.toString() + "/" + packageJsonFile)
            let jsonObj = JsonValue.fromStr(String.fromUtf8(fileContent)).asObject()
            if (let Some(v) <- jsonObj.get("types")) {
                return v.toString().replace("\"", "") // Strip the double quotes
            }
            if (let Some(v) <- jsonObj.get("typings")) {
                return v.toString().replace("\"", "") // Strip the double quotes
            }
        }
    }
    return "index.d.ts"
}

func createDir(dir: String) {
    mtx.lock()
    if (!Directory.exists(dir)) {
        Directory.create(dir, recursive: true)
    }
    mtx.unlock()
}

func getModuleName(thirdpartyName: ?String, module: String) {
    match (thirdpartyName) {
        case Some(v) => v
        case None =>
            if (module.startsWith("@ohos")) {
                module.replace("@ohos.", "")
            } else if (module.startsWith("@hms")) {
                module.replace("@hms.", "")
            } else {
                module
            }
    }
}

func getArkTSModuleName(gconfig: GlobalConfig, moduleName: String, moduleFullName: String): String {
    var arkTSModuleName = ""
    if (!cla.directoryPath.isEmpty()) {
        if (!cla.directoryPath.endsWith("/")) {
            cla.directoryPath += "/"
        }
        let dir = cla.directoryPath.split("/")
        if (dir.size >= 2) {
            arkTSModuleName = dir[dir.size - 2]
        }
    } else {
        let dir = cla.arkpath.split("/")
        if (dir.size >= 2) {
            arkTSModuleName = dir[dir.size - 2]
        }
    }
    arkTSModuleName = if (gconfig.is3rd && moduleFullName != gconfig.typeDeclFile) {
        arkTSModuleName + "_" + removeTsSuffix(moduleName)
    } else {
        arkTSModuleName
    }
    arkTSModuleName
}

func parseFile(arkpath: String, outDir: String, jsASTPath: String, gconfig: GlobalConfig,
    moduleNameTemp!: String = "", thirdpartyName!: ?String = None, relativePathName!: String = "", tsPath!: String = "") {
    let isInvalid = !File.exists(arkpath) || !arkpath.endsWith(".d.ts") && !arkpath.endsWith(".d.ets")
    if (isInvalid) {
        throw IllegalArgumentException("Invalid arkts input: ${arkpath}")
    }

    let arkFileName = if (OS == OS_WINDOWS) {
        let canonicalPath = arkpath.replace("\\", "/")
        canonicalPath.split("/")[canonicalPath.split("/").size - 1]
    } else {
        arkpath.split("/")[arkpath.split("/").size - 1]
    }

    let module = arkFileName.split("/")[0]
    let moduleFullName = module

    var moduleName = getModuleName(thirdpartyName, module)

    let jsonDir = outDir + "json/arkts/"
    createDir(jsonDir)

    // Avoid duplicate filename in different directories, append prefix to filename
    var prefix = ""
    if (relativePathName != "") {
        prefix = relativePathName.replace("/", "_") + "_"
    }

    println("========Analyzing ${arkFileName} Begins!========")
    execute("node", jsASTPath, arkpath, jsonDir, tsPath, prefix)

    var arkJson = ""
    if (arkFileName.endsWith(".d.ts")) {
        arkJson = jsonDir + prefix + arkFileName.replace("d.ts", "json")
    } else if (arkFileName.endsWith(".d.ets")) {
        arkJson = jsonDir + prefix + arkFileName.replace("d.ets", "json")
    }

    if (!File.exists(arkJson)) {
        println("Warning: ${arkJson} generation failed! ${arkFileName} is skipped.\n")
        return
    }

    //扫描整个ark json文件，将内容保存在全局变量gconfig里,gconfig的声明在globalConfig.cj
    //如若需要指定cj json，可以考虑扩容Global
    let fileContent = String.fromUtf8(File.readFrom(arkJson))
    let transform = Transform(fileContent)
    let jsonContent = JsonValue.fromStr(fileContent)
    dfs(jsonContent, ArrayList<String>(), transform)

    let outCJPath = if (gconfig.is3rd) {
        outDir
    } else {
        outDir + "cj"
    }

    createDir(outCJPath)

    var name = if (gconfig.is3rd && moduleFullName != gconfig.typeDeclFile) {
        moduleNameTemp + "_" + moduleFullName
    } else {
        moduleNameTemp
    }

    var arkTSModuleName = getArkTSModuleName(gconfig, moduleName, moduleFullName)

    gconfig.load(transform, prefix + moduleName, name, arkTSModuleName, outCJPath)
    gconfig.showConfig()
}

func getArrayFromJson(dataModel: DataModelStruct, key: String): JsonArray {
    return JsonArray.fromStr(dataModel.get(key).toJson().toString()).asArray()
}

func getFiles(fileInfos: ArrayList<FileInfo>, dir: Path): Unit {
    let files = Directory.readFrom(dir)
    for (f in files) {
        if (f.isSymbolicLink()) {
        } else if (f.isDirectory()) {
            getFiles(fileInfos, f.path)
        } else if (f.isFile()) {
            let fileName = f.path.fileName
            if (fileName.endsWith(".d.ts") || fileName.endsWith(".d.ets")) {
                fileInfos.add(f)
            }
        }
    }
}

func analyseType(dms: DataModelStruct, transform: Transform, nameSpace: NameSpace) {
    let typeArray = getArrayFromJson(dms, "type")
    for (i in 0..typeArray.size()) {
        let result = TypeType.deserialize(DataModel.fromJson(typeArray[i]))
        transform.types.add(result.name)
        nameSpace.types.add(result)
    }
}

func analyseConst(dms: DataModelStruct, transform: Transform, nameSpace: NameSpace) {
    let constArray = getArrayFromJson(dms, "variable")
    for (i in 0..constArray.size()) {
        let result = VariableType.deserialize(DataModel.fromJson(constArray[i]))
        transform.consts.add(result.name)
        nameSpace.consts.add(result)
    }
}

func analyseEnums(dms: DataModelStruct, transform: Transform, nameSpace: NameSpace) {
    let enumArray = getArrayFromJson(dms, "enum")
    for (i in 0..enumArray.size()) {
        let result = EnumType.deserialize(DataModel.fromJson(enumArray[i]))
        transform.enums.add(result.name)
        nameSpace.enums.add(result)
    }
}

func analyseClasses(dms: DataModelStruct, transform: Transform, nameSpace: NameSpace) {
    let classArray = getArrayFromJson(dms, "class")
    for (i in 0..classArray.size()) {
        let result = ObjectType.deserialize(DataModel.fromJson(classArray[i]))
        transform.classes.add(result.name)
        if (result.parents.size > 0) {
            transform.baseClasses.add(result.parents[0])
        }
        result.nameSpace = nameSpace
        nameSpace.classes[result.name] = result
        for (parent in result.parents) {
            if (!nameSpace.childrenMap.contains(parent)) {
                nameSpace.childrenMap[parent] = ArrayList<String>()
            }
            nameSpace.childrenMap[parent].add(result.name)
        }
    }
}

func analyseInterfaces(dms: DataModelStruct, transform: Transform, nameSpace: NameSpace) {
    let interArray = getArrayFromJson(dms, "interface")
    for (i in 0..interArray.size()) {
        let result = ObjectType.deserialize(DataModel.fromJson(interArray[i]))
        transform.interfaces.add(result.name)
        if (result.parents.size > 0) {
            transform.baseInterfaces.add(result.parents[0])
        }
        result.nameSpace = nameSpace
        nameSpace.interfaces[result.name] = result
        for (parent in result.parents) {
            if (!nameSpace.childrenMap.contains(parent)) {
                nameSpace.childrenMap[parent] = ArrayList<String>()
            }
            nameSpace.childrenMap[parent].add(result.name)
        }
    }
}

func analyseGlobalVariables(dms: DataModelStruct, transform: Transform, nameSpace: NameSpace) {
    let methodArray = getArrayFromJson(dms, "method")
    for (i in 0..methodArray.size()) {
        let result = MethodType.deserialize(DataModel.fromJson(methodArray[i]))
        result.namespace = nameSpace
        nameSpace.methods.add(result)
    }

    transform.namespaces.add(nameSpace)
}

func analyseImport(dms: DataModelStruct, transform: Transform) {
    let importArray = getArrayFromJson(dms, "import")
    for (i in 0..importArray.size()) {
        let dms1 = match (DataModel.fromJson(importArray[i])) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let dms2 = match (dms1.get("info")) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let result = String.deserialize(dms2.get("name"))
        transform.importInfo.add(result)
    }
}

func dfs(contents: JsonValue, track: ArrayList<String>, transform: Transform): Unit {

    //解析json
    let dm = DataModel.fromJson(JsonValue.fromStr(contents.toString()))
    let dms = (dm as DataModelStruct) ?? throw IllegalArgumentException("this data is not DataModelStruct")

    let info = HashMap<String, String>.deserialize(dms.get("info"))
    if (info.size > 0) {
        track.add(info["name"]) //track用于记录namespace层级关系
    }

    let nameSpace: NameSpace = NameSpace()
    nameSpace.spacePath = String.join(track.toArray(), delimiter: "_")

    //解析type
    analyseType(dms, transform, nameSpace);

    //解析const
    analyseConst(dms, transform, nameSpace);

    //解析enums
    analyseEnums(dms, transform, nameSpace);

    //解析classes
    analyseClasses(dms, transform, nameSpace);

    //解析interfaces
    analyseInterfaces(dms, transform, nameSpace);

    //解析全局函数
    analyseGlobalVariables(dms, transform, nameSpace);

    //解析import
    analyseImport(dms, transform);

    //解析symbols
    match (dms.get("symbols")) {
        case symbols: DataModelStruct => transform.symbols = HashMap<String, String>.deserialize(symbols)
        case _ => ()
    }

    let nameSpaceArray = getArrayFromJson(dms, "module")
    if (nameSpaceArray.size() == 0) {
        return
    }
    for (i in 0..nameSpaceArray.size()) {
        dfs(nameSpaceArray[i], track, transform)
    }
}
