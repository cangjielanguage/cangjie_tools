// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.fs.*
import std.sync.*
import std.env.*
import std.deriving.Derive
import stdx.encoding.json.*
import stdx.serialization.serialization.*

import cjpm.util.*
import cjpm.toml.*
import cjpm.toml.Decoder as DecoderToml

public let DIR_CURRENT: String = "."
public let DIR_RUN: String = "."
public let LOCK_FILE_NAME: String = "cjpm.lock"
public const TARGET: String = "target"
public const BIN: String = "bin"
public const UNITTEST_BIN: String = "unittest_bin"
public const RELEASE: String = "release"
public const MOCK: String = "mock"
public const DEBUG: String = "debug"
public const DIR_SRC: String = "src"
public const CONFIG_FILE_NAME: String = "cjpm.toml"
public const HISTORY_FILE_NAME: String = ".cjpm-history"
public const DIR_COV_OUTPUT: String = "cov_output"
public const BUILD_GCNO_OUTPUT: String = "build_gcno_output"
public const TEST_GCNO_OUTPUT: String = "test_gcno_output"
public const INCREMENTAL_CACHE: String = "incremental-cache.json"
public const WORKSPACE_NAME: String = "cjpm-workspace"
public const DEP_INCREMENTAL_CACHE: String = ".dep-cache"
public var maxParallelSize: Int64 = getCpus()
public let CUR_PARALLEL_SIZE: AtomicInt64 = AtomicInt64(0)
public var crossCompileTarget: String = ""
public var targetConfigName: String = ""
private let outputTypeToEnum: HashMap<String, OutputType> = HashMap(
    [
        ("static", Static),
        ("dynamic", Dynamic),
        ("executable", Exe),
        ("chir", Chir),
        ("obj", Obj)
    ]
)

@Derive[Equatable]
public enum OutputType <: Serializable<OutputType> & ToString {
    | Static
    | Dynamic
    | Exe
    | Chir
    | Obj
    | Unknown(String)

    // output type is considered valid if it can be used in public API by the user
    // e.g. `package.output-type` field in `cjpm.toml` or `cjpm init --type=<output-type>`
    public prop isValid: Bool {
        get() {
            match (this) {
                case Static | Dynamic | Exe => true
                case Chir => false // This output type is currently for internal usage only
                case Obj => false // This output type is currently for internal usage only
                case Unknown(_) => false
            }
        }
    }

    public prop isLib: Bool {
        get() {
            match (this) {
                case Static | Dynamic => true
                case Exe | Chir | Obj | Unknown(_) => false
            }
        }
    }

    public func toString(): String {
        match (this) {
            case Static => "static"
            case Dynamic => "dynamic"
            case Exe => "executable"
            case Chir => "chir"
            case Obj => "obj"
            case Unknown(s) => s
        }
    }

    public prop asCompilerArg: String {
        get() {
            match (this) {
                case Static => "staticlib"
                case Dynamic => "dylib"
                case Exe => "exe"
                case Chir => "chir"
                case Obj => "obj"
                case Unknown(s) => s
            }
        }
    }

    public static func fromString(s: String): OutputType {
        if (let Some(res) <- outputTypeToEnum.get(s)) {
            return res
        }
        return Unknown(s)
    }

    public func serialize(): DataModel {
        this.toString().serialize()
    }

    public static func deserialize(dm: DataModel): OutputType {
        var dmstr = match (dm) {
            case data: DataModelString => data
            case _: DataModelNull => DataModelString("")
            case _ => throw CliFailError("tried to deserialize OutputType not from the DataModelString")
        }
        return fromString(dmstr.getValue())
    }
}

// Type for running resolve()
public const RESOLVE_BUILD: String = "build"
public const RESOLVE_TEST: String = "test"
public const RESOLVE_CHECK: String = "check"
public const RESOLVE_INSTALL: String = "install"
@When[os == "Windows"]
const NEW_LINE: String = "\r\n"
@When[os != "Windows"]
const NEW_LINE: String = "\n"
@When[os == "Windows"]
const SPLIT_ENV: String = ";"
@When[os != "Windows"]
const SPLIT_ENV: String = ":"
@When[os == "Linux" || os == "macOS"]
public const WHICH: String = "which"
@When[os == "Windows"]
public const WHICH: String = "where.exe"

public func getTargetConfigName(): Option<String> {
    let (flag, output, errlog) = execWithOutput("cjc", ArrayList<String>(["-v"]))
    if (!flag) {
        eprintln(errlog)
        eprintln("Error: failed to run 'cjc -v'")
        return Option.None
    }
    if (!output.contains("Target: ")) {
        eprintln("Error: no target info in output of command 'cjc -v':")
        eprintln(output)
        return Option.None
    }
    return output.split("Target: ")[1].split(NEW_LINE)[0]
}

public class CpmState {
    private var __cjcInfo: ?CjcInfo = None
    private var __cjcPath: ?String = None
    private var __isExperimental: ?Bool = None

    init() {}

    public prop cjcInfo: CjcInfo {
        get() {
            match (__cjcInfo) {
                case None =>
                    let info = CjcInfo.findCjc()
                    __cjcInfo = info
                    info
                case Some(info) => info
            }
        }
    }

    public prop cjcPath: String {
        get() {
            match (__cjcPath) {
                case None =>
                    let info = initCjcPath()
                    __cjcPath = info
                    info
                case Some(info) => info
            }
        }
    }

    public prop isExperimental: Bool {
        get() {
            match (__isExperimental) {
                case None =>
                    let value = checkExperimental()
                    __isExperimental = value
                    value
                case Some(value) => value
            }
        }
    }

    private static func checkExperimental(): Bool {
        let tomlFile = Path(DIR_CURRENT).join(CONFIG_FILE_NAME).toString()
        if (!fileExists(tomlFile)) {
            return false
        }
        var isExperimental = false
        try (file = File(tomlFile, Read)) {
            let de = DecoderToml(file)
            let to = de.decode()
            let dms = (DataModel.fromToml(to) as DataModelStruct) ?? throw Exception()
            isExperimental = match (dms.accessDeep(["profile", "experimental"]) as DataModelBool) {
                case Some(dmbool) => dmbool.getValue()
                case _ => false
            }
        } catch (e: Exception) {
            isExperimental = false
        }
        return isExperimental
    }

    @When[os == "macOS"]
    private static func initCjcPath(): String {
        let (success, output, _) = execWithOutput(WHICH, ArrayList<String>(["cjc"]))
        if (success) {
            return output.trimAscii()
        }
        throw CliFailError(output)
    }

    @When[os != "macOS"]
    private static func initCjcPath(): String {
        throw CliFailError("cjcPath variable should only be used for macOS functions")
    }
}

public let state: CpmState = CpmState()
@When[os == "Linux" || os == "macOS"]
public const MOVE_COMMAND: String = "mv"
@When[os == "Linux" || os == "macOS"]
public const COPY_COMMAND: String = "cp"
@When[os == "Linux" || os == "macOS"]
public const REMOVE_OUTPUT: String = ">/dev/null 2>&1"

@When[os == "Linux" || os == "macOS"]
public func makeExeName(name: String): String {
    return name
}

@When[os == "Linux"]
public let PATH: String = "PATH"
@When[os == "Linux"]
public let LD_PATH: String = "LD_LIBRARY_PATH"

@When[os == "Linux"]
public func makeDylibName(file: String): String {
    return "lib" + swapOrgName(file) + ".so"
}

@When[os == "Linux"]
public func setLdLibraryPath(ldPath: HashSet<String>): String {
    var path: String = getLdLibraryConfig(ldPath)
    if (path.isEmpty()) {
        return ""
    }
    return "LD_LIBRARY_PATH=${path}$LD_LIBRARY_PATH"
}

@When[os == "Linux"]
public func getLdLibraryConfig(ldPath: HashSet<String>): String {
    var path: String = ""
    for (k in ldPath) {
        if (directoryExists(k)) {
            path += "\"${k}\":"
        }
    }
    return path
}

@When[os == "Linux"]
public func getNativeLdLibraryConfig(ldPath: HashSet<String>): String {
    var path: String = ""
    for (k in ldPath) {
        if (directoryExists(k)) {
            path += "${k}:"
        }
    }
    return path
}

@When[os == "macOS"]
public let PATH: String = "PATH"
@When[os == "macOS"]
public let LD_PATH: String = "DYLD_LIBRARY_PATH"

@When[os == "macOS"]
public func makeDylibName(file: String): String {
    return "lib" + swapOrgName(file) + ".dylib"
}

@When[os == "macOS"]
public func setLdLibraryPath(ldPath: HashSet<String>): String {
    // On certain versions of MacOS, DYLD_LIBRARY_PATH does not propagate
    // to child processes, so we need to set it manually
    let finalLdPath = ldPath.clone()
    if (let Some(pathElements) <- getVariable(LD_PATH)?.split(SPLIT_ENV, removeEmpty: true)) {
        finalLdPath.add(all: pathElements)
    }
    let path: String = getLdLibraryConfig(finalLdPath)
    if (path.isEmpty()) {
        return ""
    }
    return "${LD_PATH}=${path}$${LD_PATH}"
}

@When[os == "macOS"]
public func getLdLibraryConfig(ldPath: HashSet<String>): String {
    var path: String = ""
    for (k in ldPath) {
        if (directoryExists(k)) {
            path += "\"${k}\":"
        }
    }
    return path
}

@When[os == "macOS"]
public func getNativeLdLibraryConfig(ldPath: HashSet<String>): String {
    var path: String = ""
    for (k in ldPath) {
        if (directoryExists(k)) {
            path += "${k}:"
        }
    }
    return path
}

@When[os == "Windows"]
public const PATH_SPLIT_CHAR: String = "\\"
@When[os == "Windows"]
public const COMPILE_TOOL: String = "cjc.exe"
@When[os == "Windows"]
public const CJPM_TOOL: String = "cjpm.exe"
@When[os == "Windows"]
public const MOVE_COMMAND: String = "move"
@When[os == "Windows"]
public const COPY_COMMAND: String = "copy"
@When[os == "Windows"]
public const REMOVE_OUTPUT: String = ">nul 2>nul"
@When[os == "Windows"]
public let PATH: String = "Path"
@When[os == "Windows"]
public let LD_PATH: String = "Path"

@When[os == "Windows"]
public func makeDylibName(file: String): String {
    return "lib" + swapOrgName(file) + ".dll"
}

@When[os == "Windows"]
public func makeExeName(name: String): String {
    if (name.endsWith(".exe")) {
        return name
    }
    return name + ".exe"
}

@When[os == "Windows"]
public func setLdLibraryPath(ldPath: HashSet<String>): String {
    var path: String = getLdLibraryConfig(ldPath)
    if (path.isEmpty()) {
        return ""
    }
    return "set Path=${path}%Path% &"
}

@When[os == "Windows"]
public func getLdLibraryConfig(ldPath: HashSet<String>): String {
    var path: String = ""
    for (k in ldPath) {
        if (directoryExists(k)) {
            path += "${k};"
        }
    }
    return path
}

@When[os == "Windows"]
public func getNativeLdLibraryConfig(ldPath: HashSet<String>): String {
    var path: String = ""
    for (k in ldPath) {
        if (directoryExists(k)) {
            path += "${k};"
        }
    }
    return path
}

@When[os == "Linux" || os == "macOS"]
public let HOME_ENV: String = "HOME"
@When[os == "Windows"]
public let HOME_ENV: String = "USERPROFILE"
public let DEFAULT_CONFIG_LOCATION: String = getVariable(HOME_ENV).getOrDefault {
    eprintln("Error: no ${HOME_ENV}");
    ""
}

// posix guarantees that there must be $HOME set so no need to git valid alternative if it doesn't exist
@When[os == "Linux" || os == "macOS"]
public const PATH_SPLIT_CHAR: String = "/"
@When[os == "Linux" || os == "macOS"]
public const COMPILE_TOOL: String = "cjc"
@When[os == "Linux" || os == "macOS"]
public const CJPM_TOOL: String = "cjpm"

public let LTO_TYPE_LIST: Array<String> = ["thin", "full"]
public let COMMAND_CACHE: ArrayList<CommandInfo> = ArrayList<CommandInfo>()
public let SHOW_CACHE: ArrayList<PkgInfo> = ArrayList<PkgInfo>()
public let CJPM_COMPILE_INFO: HashMap<String, Float64> = HashMap<String, Float64>()
public let CACHE_MUTEX: Mutex = Mutex()
public var memProcess: Future<Thread> = spawn { => return Thread.currentThread}

public class PkgInfo <: Serializable<PkgInfo> {
    public var name: String = ""
    public var pid: String = "CJPM"
    public var tid: Int64 = 0
    public var ph: String = ""
    public var ts: Int64 = 0

    public init() {}

    public init(name: String, ph: String, ts: Int64) {
        this.name = name
        this.ph = ph
        this.ts = ts
    }

    public func serialize(): DataModel {
        return DataModelStruct()
            .add(field<String>(PACKAGE_NAME, this.name))
            .add(field<String>("pid", this.pid))
            .add(field<String>("tid", this.tid.toString()))
            .add(field<String>("ph", this.ph))
            .add(field<String>("ts", this.ts.toString()))
    }

    public static func deserialize(_: DataModel): PkgInfo {
        return PkgInfo()
    }
}

public class CommandInfo <: Serializable<CommandInfo> {
    public var pkgName: String = ""
    public var pkgCommand: String = ""

    public init() {}

    public init(pkgName: String, pkgCommand: String) {
        this.pkgName = pkgName
        this.pkgCommand = pkgCommand
    }

    public func serialize(): DataModel {
        return DataModelStruct()
            .add(field<String>("pkgName", this.pkgName))
            .add(field<String>("pkgCommand", this.pkgCommand))
    }

    public static func deserialize(_: DataModel): CommandInfo {
        return CommandInfo()
    }
}

public var GLOBAL_CJPM_CONFIG_DIR: Path = Path(getVariable("CJPM_CONFIG") ??
    DEFAULT_CONFIG_LOCATION + PATH_SPLIT_CHAR + ".cjpm")

public let FINAL_CJPM_CONFIG_INFO: SettingsConfig = loadConfigInfo()

public open class CommandConfig {
    public var isCov: Bool = false
    public var isVerbose: Bool = false
    public var isCrossCompile: Bool = false
    public var targetDir: String = ""
    public var customizedOption: HashSet<String> = HashSet<String>()
    public var isLto: Bool = false
    public var ltoValue: String = ""
    public var isDebug: Bool = false
    public var memberModule: String = ""
}

@Derive[Hashable, Equatable]
public enum CompilePhase {
    ObjOnly | LinkOnly | Default

    public func toString(): String {
        match (this) {
            case ObjOnly => "ObjOnly"
            case LinkOnly => "LinkOnly"
            case Default => "Default"
        }
    }
}

@Derive[Hashable, Equatable]
public struct TaskIdentifier {
    public TaskIdentifier(
        public let fullName: String,
        public let target: String,
        public let sourceSetDir: String,
        public let compilePhase!: CompilePhase = Default
    ) { }
}

public struct PackageList {
    private let packages: Array<ResolveItem>
    private let macroSet: HashSet<String>
    private let dylibSet: HashSet<String>
    private let exeSet: HashSet<String>
    private let testSet: HashSet<String>

    public let all: Array<String>
    public let macros: Array<String>
    public let dylib: Array<String>
    public let exe: Array<String>
    public let test: Array<String>
    public let productSuffixes: ReadOnlyMap<String, String>

    public init(packages: Iterable<ResolveItem>) {
        this.packages = packages |> collectArray

        let capacity = this.packages.size
        let all = ArrayList<String>(capacity)
        let dylib = ArrayList<String>(capacity)
        let macros = ArrayList<String>(capacity)
        let exe = ArrayList<String>(capacity)
        let test = ArrayList<String>(capacity)

        for (pkg in this.packages) {
            let name = pkg.fullName
            all.add(name)

            if (pkg.isMacroPackage) {
                macros.add(name)
            }
            if (pkg.isPureTestPkg()) {
                test.add(name)
            }
            match (pkg.outputType) {
                case Exe => exe.add(name)
                case Dynamic => dylib.add(name)
                case Static | Chir | Obj | Unknown(_) => ()
            }
        }

        this.all = all.toArray()
        this.macros = macros.toArray()
        this.dylib = dylib.toArray()
        this.exe = exe.toArray()
        this.test = test.toArray()
        this.productSuffixes = makeProductSuffixes(packages)

        this.macroSet = HashSet<String>(this.macros)
        this.dylibSet = HashSet<String>(this.dylib)
        this.exeSet = HashSet<String>(this.exe)
        this.testSet = HashSet<String>(this.test)
    }

    init() {
        this([])
    }

    public func isMacro(name: String): Bool {
        this.macroSet.contains(name)
    }

    public func isDyLib(name: String): Bool {
        this.dylibSet.contains(name)
    }

    public func isExe(name: String): Bool {
        this.exeSet.contains(name)
    }

    public func isTest(name: String): Bool {
        this.testSet.contains(name)
    }

    public func bestFitFeatures(features: HashSet<Feature>): HashMap<String, String> {
        packages |> filterMap {
            it: ResolveItem => if (!it.isMultiplatform) {
                return None<(String, String)>
            } else {
                let reversed = it.packagePath._sources.clone()
                reversed.reverse()
                for (source in reversed) {
                    let finalFeatures = it.featureDeducer.addFeature(features).collect()
                    if (finalFeatures.contains(all: source.features)) {
                        return Some((it.fullName, getPath(source.outputSuffix.toString())))
                    }
                }
                return None<(String, String)>
            }
        } |> collectHashMap
    }

    private static func makeProductSuffixes(packages: Iterable<ResolveItem>): HashMap<String, String> {
        packages |> filterMap { it: ResolveItem =>
            let productEntry = it.packagePath._sources.last ?? return None<(String, String)>
            return Some((it.fullName, productEntry.outputSuffix.toString()))
        } |> collectHashMap
    }

}

public enum CjcParallelConfig {
    | Default
    | Heuristic
    | Spicific(Int64)
}

public class BuildConfig <: CommandConfig {
    public var globalConfig: GlobalConfig = GlobalConfig()

    public var isCompilePipelineParallel: Bool = false
    public var isIncremental: Bool = false
    public var isRebuild: Bool = false
    public var isLint: Bool = false
    public var isInstall: Bool = false
    public var curModuleName: String = WORKSPACE_NAME
    public var lastEndTime: Int64 = 0
    public var cjcJobs: CjcParallelConfig = CjcParallelConfig.Default
    public var packageList: PackageList = PackageList()
    public var pkgRequireByMacro = HashSet<String>()
    public var requiredForTests: Bool = false
    public var requiresMap: HashMap<String, HashSet<String>> = HashMap<String, HashSet<String>>()
    public var rebuildList: HashSet<String> = HashSet<String>()
    public var rebuildMacros: HashSet<String> = HashSet<String>()
    public var indirectRebuilds: HashSet<String> = HashSet<String>()
    // Record json cache for each module
    public var jsonInformation: HashMap<String, ArrayList<String>> = HashMap<String, ArrayList<String>>()
    // Record package cache for each module
    public var pkgCacheMap: HashMap<String, CacheItem> = HashMap<String, CacheItem>()
    // Record the module name that has changed json data
    public var changedModuleList: ArrayList<String> = ArrayList<String>()
    // Record the cjo timetamp
    public var cjoTimeMap: HashMap<String, Int64> = HashMap<String, Int64>()
    public var mockSupported: Bool = false
    public var originTargetPath: String = TARGET
    public var exeName: String = "main"
    public var binDepMap: HashMap<String, BinDeps> = HashMap<String, BinDeps>()
    public var crossBinDepMap: HashMap<String, BinDeps> = HashMap<String, BinDeps>()
    public var targetMacroPkgs: HashSet<String> = HashSet<String>() // used by cross compile
    public var targetMacroMods: HashSet<String> = HashSet<String>() // used by cross compile
    public var hasSubPkgs: HashSet<String> = HashSet<String>() // record the lib packages that have subPkgs.

    // External config
    public var scriptConfig: ScriptConfig = ScriptConfig()
    public var superPkgMap: HashMap<String, SuperPackageConfig> = HashMap<String, SuperPackageConfig>()
    public var superPkgSubMap: HashMap<String, String> = HashMap<String, String>()

    public let resolveItemMap: HashMap<String, ResolveItem> = HashMap<String, ResolveItem>()

    public func getOptionCacheData(): ArrayList<String> {
        var cacheData = ArrayList<String>()
        if (isCov) {
            cacheData.add("--coverage")
        }
        cacheData.add(all: customizedOption)
        return cacheData
    }
}

public class TestConfig <: CommandConfig {
    public var globalConfig: GlobalConfig = GlobalConfig()

    public var noRun: Bool = false
    public var skipBuild: Bool = false
    public var isBench: Bool = false
    public var dryRun: Bool = false
    public var filters: ?String = None
    public var includeTags: ?String = None
    public var excludeTags: ?String = None
    public var noColor: Bool = false
    public var timeoutEach: ?String = None
    public var nWorkers: ?String = None
    public var showTags: Bool = false
    public var noCaptureOutput: Bool = false
    public var showAllOutput: Bool = false
    public var randomSeed: ?String = None
    public var isSpecifiedConcurrency: Bool = false
    public var testModule: HashSet<String> = HashSet<String>()
    public var packagesToTest: HashSet<PackageFqName> = HashSet<PackageFqName>()
    public var packagePathsToTest: HashSet<String> = HashSet<String>()
    public var isModuleTest: Bool = true
    public var reportPath: ?String = None
    public var baselinePath: ?String = None
    public var reportFormat: ?String = None
    public var mock: MockSupport = MockSupport.On
    public var profile: TestProfile = TestProfile(JsonObject())
    public var originTargetPath: String = TARGET
    public var envConfig: HashMap<String, EnvInfo> = HashMap<String, EnvInfo>()
    public var isIncremental: Bool = false
    public var rebuildList: HashSet<String> = HashSet<String>()
    public var testPkgs: HashSet<String> = HashSet<String>()
    public var packageDepMap: HashMap<String, ResolveItem> = HashMap<String, ResolveItem>()
    public var incrementalCache: HashMap<String, HashMap<String, String>> = HashMap<String, HashMap<String, String>>()

    // dynamic progress
    public var isProgressReportDisabled: Bool = false
    public var isProgressReportBrief: Bool = false
    public var progressReportEntriesLimit: ?Int64 = None
    public var superPkgSubMap: HashMap<String, String> = HashMap<String, String>()

    public prop origCommand: String {
        get() {
            if (isBench) {
                "bench"
            } else {
                "test"
            }
        }
    }
}

public class RunConfig {
    public var globalConfig: GlobalConfig = GlobalConfig()

    public var isVerbose: Bool = false
    public var skipBuild: Bool = false
    public var isDebug: Bool = false
    public var rootPkgName: String = ""
    public var name: String = "main"
    public var buildArgs: String = ""
    public var runArgs: String = ""
    public var enableFeatures: String = ""
    public var newRunArgs: ArrayList<String> = ArrayList<String>()
    public var filePath: String = ""
    public var targetPath: String = ""
    public var originTargetPath: String = TARGET
    public var envConfig: HashMap<String, EnvInfo> = HashMap<String, EnvInfo>()
}

public struct TreeItem {
    public TreeItem(
        public let packagePath: CJMPSource,
        public let version: String,
        public let requires: ReadOnlySet<String>
    ) {}
}

public class TreeConfig {
    public var globalConfig: GlobalConfig = GlobalConfig()

    public var noTest: Bool = false
    public var isVerbose: Bool = false
    public var defaultPkg: String = ""
    public var invertPkg: String = ""
    public var depth: Int32 = -1
    public var deps: HashMap<String, TreeItem> = HashMap<String, TreeItem>()
    public var treePkgs: HashSet<String> = HashSet<String>()
}

public struct PackageInfo {
    public PackageInfo(
        public let sources: CJMPSource,
        public let hashCode: String,
        public let timeStamp: Int64,
        public let hasTestFiles: Bool,
        public let hasProdFiles: Bool
    ) {}
}

public class SuperPackageConfig {
    public SuperPackageConfig(
        public let pkgName: String,
        public let outputType: OutputType
    ) {}

    public let subPkgSet: HashSet<String> = HashSet<String>()
}

public enum InstallType {
    | Default | Repo | Git | Local
}

public class GitConfig {
    public var url: String = ""
    public var branch: String = ""
    public var tag: String = ""
    public var commitId: String = ""
}

public class InstallConfig {
    public var globalConfig: GlobalConfig = GlobalConfig()

    // common
    public var list: Bool = false
    public var installType: InstallType = InstallType.Default
    public var memberModule: String = ""
    public var version: String = ""
    public var debug: Bool = false
    public var verbose: Bool = false
    // central repository config
    public var artifacts: ArrayList<String> = ArrayList<String>()
    // git config
    public var gitConfig: GitConfig = GitConfig()
    // local config
    public var localPath: String = ""
    public var skipBuild: Bool = false
    // build config
    public var targetDir: String = TARGET
    public var customizedOption: HashSet<String> = HashSet<String>()
    public var installName: String = ""
    // output root
    public var root: String = ""
    public var rootBin: String = ""
    public var rootRepo: String = ""
    public var rootLibs: String = ""
    public var rootList: String = ""
}

public class BundleConfig {
    public var isVerbose: Bool = false
    public var isTest: Bool = true
    public var isLint: Bool = true
    public var currentDir: String = DIR_CURRENT
    public var targetDir: String = TARGET
    public var scriptDir: String = BUILD_SCRIPT_DIR

    public init() {}

    public init(isVerbose: Bool) {
        this.isVerbose = isVerbose
    }
}

public class ScriptConfig {
    public var moduleName: String = ""
    public var modulePath: String = ""
    public var moduleSrcPath: String = ""
    public var scriptPath: String = ""
    public var scriptTargetDir: String = ""
    public var scriptExe: String = ""
    public var scriptHash: String = ""
    public var scriptLog: String = ""
    public var scriptDep: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    public var scriptLdPath: HashSet<String> = HashSet<String>()
    public var isVerbose: Bool = false
    public var isDebug: Bool = false
    public var isCrossCompile: Bool = false
    public var isRelocated: Bool = false

    public init() {}
    public init(scriptConfig: ScriptConfig) {
        this.moduleName = scriptConfig.moduleName
        this.modulePath = scriptConfig.modulePath
        this.moduleSrcPath = scriptConfig.moduleSrcPath
        this.scriptPath = scriptConfig.scriptPath
        this.scriptTargetDir = scriptConfig.scriptTargetDir
        this.scriptExe = scriptConfig.scriptExe
        this.scriptHash = scriptConfig.scriptHash
        this.scriptLog = scriptConfig.scriptLog
        this.scriptDep = scriptConfig.scriptDep
        this.isVerbose = scriptConfig.isVerbose
        this.isDebug = scriptConfig.isDebug
        this.isCrossCompile = scriptConfig.isCrossCompile
        this.isRelocated = scriptConfig.isRelocated
    }

    public func isEmpty(): Bool {
        return this.moduleName.isEmpty()
    }
}

public class Stack<T> {
    let elem: ArrayList<T> = ArrayList<T>()

    public func top(): Option<T> {
        if (this.elem.isEmpty()) {
            return Option.None
        }
        return this.elem[this.elem.size - 1]
    }

    public func push(element: T): Unit {
        this.elem.add(element)
        return
    }

    public func pop(): Bool {
        if (this.elem.isEmpty()) {
            return false
        }
        this.elem.remove(at: this.elem.size - 1)
        return true
    }
}

public class GlobalConfig {
    public var isVerbose: Bool = false
    public var isDebug: Bool = false
    public var isTest: Bool = false
    public var isInstall: Bool = false
    public var isCross: Bool = false
    public var isIncremental: Bool = false
    public var isForScript: Bool = false
    public var isSkipScript: Bool = false
    public var isProjectCombine: Bool = false
    public var isAnalysisCompilePerformance: Bool = false
    public var rootName: String = WORKSPACE_NAME
    public var resolveType: String = RESOLVE_CHECK
    public var rootPath: String = DIR_CURRENT
    public var modulePath: String = DIR_CURRENT
    public var targetDir: String = TARGET
    public var compilePerformanceTargetDir: String = ""
    public var originDir: String = ""
    public var nativeDir: String = ""
    public var lockFile: String = LOCK_FILE_NAME
    public var scriptDir: String = DIR_CURRENT
    public var scriptList: ArrayList<ScriptConfig> = ArrayList<ScriptConfig>()
    public var cjPathList: ArrayList<String> = ArrayList<String>()
    public var cLibLinkMap: HashMap<String, ArrayList<String>> = HashMap<String, ArrayList<String>>()
    public var crossCLibLinkMap: HashMap<String, ArrayList<String>> = HashMap<String, ArrayList<String>>()
    public var ldPath: HashSet<String> = HashSet<String>()
    public var overrideCompileOption: String = ""
    public var nativeOverrideOption: String = ""
    public var customizedOption: HashSet<String> = HashSet<String>()
    public var noFeatureDeduce: Bool = false
    public var enabledFeatures: Array<Feature> = []
    public var verifiedEnabledFeatures: Array<Feature> = []

    // record git dependency: (url, commitId, localPath)
    public let gitStack: Stack<(String, String)> = Stack<(String, String)>()

    public init() {}
    public init(globalConfig: GlobalConfig) {
        this.isVerbose = globalConfig.isVerbose
        this.isDebug = globalConfig.isDebug
        this.isTest = globalConfig.isTest
        this.isInstall = globalConfig.isInstall
        this.isCross = globalConfig.isCross
        this.isIncremental = globalConfig.isIncremental
        this.isForScript = globalConfig.isForScript
        this.isSkipScript = globalConfig.isSkipScript
        this.isProjectCombine = globalConfig.isProjectCombine
        this.resolveType = globalConfig.resolveType
        this.rootName = globalConfig.rootName
        this.rootPath = globalConfig.rootPath
        this.modulePath = globalConfig.modulePath
        this.targetDir = globalConfig.targetDir
        this.originDir = globalConfig.originDir
        this.nativeDir = globalConfig.nativeDir
        this.lockFile = globalConfig.lockFile
        this.scriptDir = globalConfig.scriptDir
        this.cjPathList.add(all: globalConfig.cjPathList)
        this.ldPath.add(all: globalConfig.ldPath)
        this.cLibLinkMap = globalConfig.cLibLinkMap.clone()
        for (scriptConfig in globalConfig.scriptList) {
            this.scriptList.add(ScriptConfig(scriptConfig))
        }
        this.overrideCompileOption = globalConfig.overrideCompileOption
        this.nativeOverrideOption = globalConfig.nativeOverrideOption
        this.noFeatureDeduce = globalConfig.noFeatureDeduce
        this.enabledFeatures = globalConfig.enabledFeatures
        this.verifiedEnabledFeatures = globalConfig.verifiedEnabledFeatures
    }
}

// Self-sufficient compilation unit. 
// Stores all of the information for constructing a single `cjc` call
public struct CompileTask {
    public CompileTask(
        public let id!: TaskIdentifier,
        // The directory, of current `target`
        public let targetDir!: String = "",
        // The value of `--target-dir`: where to dump the compilation result (depends on many factors)
        public let targetPath!: String = "", 
        // The value of `-p`: The location of package files of current source set
        public let packagePath!: String = "", 
        // Is this a product source set (`--output-type` can be of binary)
        public let product!: Bool = false, 
        // Source set directory without the module path prefix. Uniquely identifies source set
        public let sourceSetDir!: String = "", 
        // The feature set header of the package
        public let sourceSetFeatures!: HashSet<Feature> = HashSet(), 
        // e.g. for `cjpm.config` the value would be `cjpm`
        public let rootPkgName!: String = "", 
        // Full package name, e.g. `cjpm.config`
        public let fullName!: String = "", 
        // The expected name of output file without extension
        public let filename!: String = "", 
        // The value of `--output-type`
        public let outputType!: OutputType = Unknown(""), 
        // The target triple passed to `--target` for compilation/cross-compilation
        public let target!: String = "", 
        // `true` if value stored in `target` field is different from host
        public let isCrossCompile!: Bool = false, 
        // A set of option fields from `cjpm.toml` of a module, specialized for this particular package
        public let compileOption!: String = "", 
        public let overrideOption!: String = "", 
        public let linkOption!: String = "", 
        public let customizedOption!: ArrayList<String> = ArrayList(), 
        // Stores names of all subpackages of a module to combine them into a single compilation unit
        public let subPkgSet!: HashSet<String> = HashSet(), 
        // Use `--whole-archive` to link dendencies
        public let isCombined!: Bool = false,
        // Compile this root package and make it into a single linkable binary
        public let isProjectCombined!: Bool = false,
        // Package is being compiled by `cjpm test`
        public let requiredForTests!: Bool = false, 
        // Include some declarations from testfiles `*_test.cj` of a package.
        public let exportForTests!: Bool = false, 
        // Incoming edges of dependency graph
        public let requireTasks!: HashSet<TaskIdentifier> = HashSet(), 
        // All features, asked by different modules of this package. Required for proper selection of product source set
        public let allEnabledFeatures!: HashSet<Feature> = HashSet(), 
        // Mappings from `[[feature]]` array of tables from `cjpm.toml`. Used to provide correct `*.cjo` files in `--import-path`
        public let featureMapping!: FeatureDeducer = FeatureDeducer.empty, 
        // Does this compiler call support AnalysisCompilePerformance
        public let isAnalysisCompilePerformance!: Bool = false, 
        // `true` if:
        //  - this is a parent package of some subpackage
        //  - requiredForTests
        public let hasSubPkgs!: Bool = false, 
        // Is this compilation task for debug build
        public let isDebug!: Bool = false,
        // Compile package with support of `coverage`
        public let isCov!: Bool = false,
        // Compile package with support of `mock`
        public let mockSupported!: Bool = false,
        // Is package a macro package
        public let isMacro!: Bool = false, 
        // A value of `--lto` flag to be passed to a compiler
        public let isLto!: Bool = false,
        public let ltoValue!: String = "",
        // 0 - CHIR file with declarations to be merged for multiplatform build
        // 1 - The value of `--common-part-cjo` for multiplatform build
        public let prevStageArtifacts!: ?(String, String) = None, 
        // Whether the source code location is spread between multiple [[source-set]] src-dir
        public let isMultiplatform!: Bool = false, 
        public let checkDepCjo!: Bool = false,
        // A list of `-Woff` compiler options
        public let supressed!: ArrayList<String> = ArrayList<String>(),
        // A file for verbose output of `cjpm` when compiling task
        public let outLogFile!: String = "", 
        // A joined file containing `stderr` of a compiler together with `cjpm` errors
        public let errLogFile!: String = "",
        // The value of `--compile-target`
 	    public let compileTargetOutput!: ?OutputType = None,
        // The *.o/obj file generated by a ObjOnly task, as the input of a LinkOnly task
 	    public let prevStageObj!: ?String = None
    ) { }
}

public func linkPackage(libName: String, libPath: String, isLto: Bool): ArrayList<String> {
    let processdLibName = swapOrgName(libName)
    let res = ArrayList<String>()
    let ltoFile = Path(libPath).join(makeLtoName(processdLibName)).toString()
    if (isLto) {
        if (fileExists(ltoFile)) {
            res.add(ltoFile)
            return res
        }
    }

    if (fileExists(Path(libPath).join(makeCangjieStaticlibName(processdLibName))) || 
        fileExists(Path(libPath).join("lib${processdLibName}.so")) || 
        fileExists(Path(libPath).join("lib${processdLibName}.dll")) || 
        fileExists(Path(libPath).join("lib${processdLibName}.dylib"))) {
        res.add(all: ["-L", libPath, "-l${processdLibName}"])
    } else if (fileExists(ltoFile)) {
        res.add(ltoFile)
    }

    return res
}

@When[os == "Linux" || os == "Windows"]
public func forcedLinkPackage(libName: String, libPath: String, isLto: Bool): ArrayList<String> {
    return linkPackage(libName, libPath, isLto)
}

@When[os == "macOS"]
public func forcedLinkPackage(libName: String, libPath: String, isLto: Bool): ArrayList<String> {
    var res = ArrayList<String>()
    let staticLibPath = Path(libPath).join(makeCangjieStaticlibName(libName)).toString()
    let dyLibPath = Path(libPath).join("lib${libName}.dylib").toString()
    if (fileExists(staticLibPath)) {
        res.add(staticLibPath)
    } else if (fileExists(dyLibPath)) {
        res.add(all: linkPackage(libName, libPath, isLto))
    }
    return res
}

@When[os == "Windows"]
public func isPackageExist(fileName: String, filePath: String, logFlag: Bool, path: String, _: Bool): Bool {
    let processdFileName = swapOrgName(fileName)
    let dylib: String = Path(filePath).join("lib${processdFileName}.so").toString()
    let staticlib: String = Path(filePath).join("lib${processdFileName}.a").toString()
    let winDylib: String = Path(filePath).join("lib${processdFileName}.dll").toString()
    let macDylib: String = Path(filePath).join("lib${processdFileName}.dylib").toString()
    if (fileExists(dylib) || fileExists(staticlib) || fileExists(winDylib) || fileExists(macDylib)) {
        return true
    }

    if (logFlag) {
        eprintln("Error: can not find the library '${fileName}' which is listed in ${path}")
    }
    return false
}

@When[os == "Linux" || os == "macOS"]
public func isPackageExist(fileName: String, filePath: String, logFlag: Bool, path: String, _: Bool): Bool {
    let processdFileName = swapOrgName(fileName)
    let ltoFile: String = Path(filePath).join("${processdFileName}.bc").toString()
    let dylib: String = Path(filePath).join("lib${processdFileName}.so").toString()
    let staticlib: String = Path(filePath).join("lib${processdFileName}.a").toString()
    let winDylib: String = Path(filePath).join("lib${processdFileName}.dll").toString()
    let macDylib: String = Path(filePath).join("lib${processdFileName}.dylib").toString()
    if (fileExists(dylib) || fileExists(staticlib) || fileExists(winDylib) || fileExists(ltoFile) || fileExists(
        macDylib)) {
        return true
    }

    if (logFlag) {
        eprintln("Error: can not find the library '${fileName}' which is listed in ${path}")
    }
    return false
}

public func isSuperPackageExist(fileName: String, filePath: String, isCrossBin: Bool): Bool {
    return isDyLibExist(fileName.split(".")[0], filePath, isCrossBin)
}

@When[os == "Linux"]
public let DYLIB_POSTFIX: String = ".so"
@When[os == "Windows"]
public let DYLIB_POSTFIX: String = ".dll"
@When[os == "macOS"]
public let DYLIB_POSTFIX: String = ".dylib"

public func isDyLibExist(pkgName: String, path: String, isCrossBin: Bool): Bool {
    let processedPkgName = swapOrgName(pkgName)
    if (!isCrossBin) {
        return fileExists(Path(path).join("lib${processedPkgName}${DYLIB_POSTFIX}").toString())
    }
    if (crossCompileTarget.contains("darwin") || crossCompileTarget.contains("apple")) {
        return fileExists(Path(path).join("lib${processedPkgName}.dylib").toString())
    } else if (crossCompileTarget.contains("windows") || crossCompileTarget.contains("w64")) {
        return fileExists(Path(path).join("lib${processedPkgName}.dll").toString())
    } else {
        return fileExists(Path(path).join("lib${processedPkgName}.so").toString())
    }
}

public func isCLibExist(fileName: String, filePath: String, path: String): Bool {
    let dylib: String = Path(filePath).join("lib${fileName}.so").toString()
    let staticlib: String = Path(filePath).join("lib${fileName}.a").toString()
    let winDylib: String = Path(filePath).join("lib${fileName}.dll").toString()
    let macDylib: String = Path(filePath).join("lib${fileName}.dylib").toString()

    if (fileExists(dylib) || fileExists(staticlib) || fileExists(winDylib) || fileExists(macDylib)) {
        return true
    }

    eprintln("Error: can not find the library '${fileName}' which is listed in ${path}")
    return false
}
