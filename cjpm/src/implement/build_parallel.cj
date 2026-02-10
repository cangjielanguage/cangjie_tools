// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.collection.concurrent.*
import std.convert.*
import std.deriving.Derive
import std.env.getVariables
import std.fs.*
import std.regex.*
import std.sync.*
import std.time.*

import stdx.serialization.serialization.*
import stdx.encoding.json.*

import cjpm.config.*
import cjpm.util.*

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
foreign func initFSWatcher(dirs: CString, cb: CFunc<(CString) -> Unit>, verbose: Bool): Bool

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
foreign func startFSWatcher(verbose: Bool): Unit

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
foreign func stopFSWatcher(verbose: Bool): Unit

type TaskMap = HashMap<TaskId, CompileTask>
type DownStreamTasks = HashMap<TaskId, HashSet<TaskId>>
type Task2SyncCounter = HashMap<TaskId, SyncCounter>

// Self-sufficient compilation unit. 
// Stores all of the information for constructing a single `cjc` call
public struct CompileTask {
    public CompileTask(
        public let id!: TaskId,
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
        public let requireTasks!: HashSet<TaskId> = HashSet(), 
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
        // Locations of `*.chir` and `*.cjo` of each source set, the current one depends on
        // 0 - CHIR file with declarations to be merged for multiplatform build
        // 1 - The value of `--common-part-cjo` for multiplatform build
        public let prevStageArtifacts!: Array<(String, String)> = Array(), 
        // Whether the source code location is spread between multiple [[source-set]] src-dir
        public let isMultiplatform!: Bool = false, 
        // A list of `-Woff` compiler options
        public let supressed!: ArrayList<String> = ArrayList<String>(),
        // The directory, where `outLogFile` and `errLogFile` are stored
        public let logPath!: String = "",
        // A file for verbose output of `cjpm` when compiling task
        public let outLogFile!: String = "", 
        // A joined file containing `stderr` of a compiler together with `cjpm` errors
        public let errLogFile!: String = "",
        // The value of `--compile-target`
 	    public let compileTargetOutput!: ?OutputType = None,
        // Default - compiling as single `cjc` call
        // ObjOnly - `cjc` call with `--output-type=obj`. Produces `*.o/obj` files
        // LinkOnly - `cjc` call which links result in a single binary. Takes `prevStageObj` as input
        public let compilePhase!: CompilePhase = Default,
        // The *.o/obj file generated by a ObjOnly task, as the input of a LinkOnly task
 	    public let prevStageObj!: ?String = None
    ) { }
}

enum Notification {
    | CjoReady
    | ChirReady
    | ObjReady
    | ExeReady
    | DylibReady
    | StaticlibReady
    | Err

    public func toString(): String {
        match (this) {
            case CjoReady => "CjoReady"
            case ChirReady => "ChirReady"
            case ObjReady => "ObjReady"
            case ExeReady => "ExeReady"
            case DylibReady => "DylibReady"
            case StaticlibReady => "StaticlibReady"
            case Err => "Err"
        }
    }
}

let OUTPUTTYPE2NOTIFICATION = HashMap<String, Notification>(
    [
        ("static", StaticlibReady),
        ("dynamic", DylibReady),
        ("executable", ExeReady),
        ("chir", ChirReady),
        ("obj", ObjReady)
    ]
)

func parallelBuild(module: ModuleResolve, buildConfig: BuildConfig): Bool {
    let engine = IncrementalEngine(
        cacheLocation: Path(buildConfig.globalConfig.targetDir).join(INCREMENTAL_CACHE)
    )

    // The `--output-type` for `exe` files is set to `staticlib` when compiling it via `cjpm test`
    // So the output type has to be a part of task identifier
    let productOutputTypes = HashMap<String, OutputType>()

    // modules.resolves is topologically sorted
    let (taskMap, orderedTasks) = module.resolves.fold((TaskMap(), ArrayList<TaskId>())) {
        acc, r: ResolveItem =>
        if (!r.hasProdFiles) {
            return acc
        }
        let compileTasks = createPackageCompileTasks(r, buildConfig, productOutputTypes)
        for (compileTask in compileTasks) {
            if (compileTask.product) {
                productOutputTypes.add(compileTask.fullName, compileTask.outputType)
            }
            registerReadSourceTask(engine, compileTask, buildConfig)
            registerReadCompilerOptionsTask(engine, compileTask, buildConfig)
            registerCompilerCallTask(engine, compileTask, buildConfig)
        }
        acc
    }
    
    engine.askAll()

    buildConfig.rebuildList = engine.dirtyTasks.iterator().filterMap { it =>
        match (it) {
            case Compile(fullName, _, _, _, _, _, _) => fullName
            case _ => None
        }
    } |> collectHashSet

    if (!buildConfig.rebuildList.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let result = if (buildConfig.isIncremental) {
        engine.execute()
    } else {
        engine.executeClean()
    }

    return match(result) {
        case Rebuild | Cached => true
        case Fatal => false
    }
}

// This functions creates an list of files, expected to be produced after compiler call
// Then different kinds of checks are performed over each file to choose whether to:
// - run task, which calls compiler
// - reuse artifacts from previous run
private func createArtifactGroup(compileTask: CompileTask, buildConfig: BuildConfig): ArtifactGroup {
    ArtifactGroup(
    match (compileTask.outputType) {
        case Static =>
            let name = if (compileTask.isLto) {
                makeLtoName(compileTask.filename)
            } else {
                makeCangjieStaticlibName(compileTask.filename)
            }
            [
                Artifact(Path(compileTask.targetPath).join(name).toString(),
                    CheckExists),
                Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                    CheckTimestamp)
            ]
        case Dynamic =>
            if (compileTask.isMacro) {[
                Artifact(Path(compileTask.targetPath).join(makeTargetMacroName(compileTask.filename, compileTask.target)).toString(),
                    CheckTimestamp),
                Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                    CheckTimestamp)
            ]} else {[
                Artifact(Path(compileTask.targetPath).join(makeTargetDylibName(compileTask.filename, compileTask.target)).toString(),
                    CheckExists),
                Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                    CheckTimestamp)
            ]}
        case Exe => [
            Artifact(Path(compileTask.targetPath).join(makeTargetExeName(compileTask.filename, compileTask.target)).toString(),
                CheckExists),
            Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                CheckTimestamp)
        ]
        case Chir => [
            Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.chir").toString(),
                CheckTimestamp),
            Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                CheckTimestamp)
        ]
        case Obj => [
            Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.o").toString(),
                CheckTimestamp),
            Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                CheckTimestamp)
        ]
        case Unknown(_) => []
    })
}

// Create task which computes the hash of source files in the directory without `*_test.cj` files
private func registerReadSourceTask(engine: IncrementalEngine, compileTask: CompileTask, buildConfig: BuildConfig): Unit {
    let id = TaskId.ReadPackageSource(compileTask.fullName, compileTask.sourceSetDir)
    engine.taskContext.createTask(
        id: id,
        dependencies: [],
        comp: { => PackageSource(Path(compileTask.packagePath)) },
        cache: engine.cache
    )
}

// Create task which stores the hash code of compiler options, which are not affecting the stored location of the binary
private func registerReadCompilerOptionsTask(engine: IncrementalEngine, compileTask: CompileTask, buildConfig: BuildConfig): Unit {
    let id = TaskId.ReadCompilerOptions(compileTask.fullName, compileTask.target, compileTask.isDebug, compileTask.mockSupported, compileTask.sourceSetDir)
    engine.taskContext.createTask(
        id: id,
        dependencies: [],
        comp: { => 
            let res = ArrayList<String>()
            res.add(all: appendCommandOptions(compileTask))
            for (option in compileTask.customizedOption) {
                res.add(all: extractOptionByString(option))
            }
            res.add(all: extractOptionByString(compileTask.compileOption))
            res.add(all: extractOptionByString(compileTask.overrideOption))
            res.add(all: appendLinkOption(compileTask))
            res.add(all: getTypeCommand(compileTask))
            CompilerOptions.fromString(res |> collectString(delimiter: " "))
        },
        cache: engine.cache
    )
}

// Create task of compiler call, which returns the locations of expected to be produced artifacts
private func registerCompilerCallTask(engine: IncrementalEngine, compileTask: CompileTask, buildConfig: BuildConfig) {
    let id = compileTask.id
    let artifacts = createArtifactGroup(compileTask, buildConfig)
    let task = engine.taskContext.createTask(
        id: id,
        dependencies: compileTask.requireTasks.toArray(),
        comp: { => 
            if (!createDirectory(compileTask.targetPath)) {
                throw Exception("Failed to create directory: `${compileTask.targetPath}`")
            }
            if (!createDirectory(compileTask.logPath)) {
                throw Exception("Failed to create directory: `${compileTask.logPath}`")
            }
            let cjcCall = constructCjcInvocation(compileTask, buildConfig)
            let envBuilder = EnvironmentBuilder()
            envBuilder.prepend(LD_PATH, buildConfig.globalConfig.ldPath)
            if (!callCompiler(compileTask, cjcCall, envBuilder, buildConfig)) {
                throw Exception("Failed to run task: `${compileTask.id}`")
            }
            return artifacts
        },
        cache: engine.cache
    )
    task.logFiles = (compileTask.outLogFile, compileTask.errLogFile)
}

// TASK OUTPUTS
private struct PackageSource <: Serializable<PackageSource> {
    PackageSource(
        let dirpath: Path
    ) { }

    public func serialize(): DataModel {
        dirpath.toString().serialize()
    }

    public static func deserialize(dm: DataModel): PackageSource {
        let dmstr = dm as DataModelString ?? throw DataModelException("this data is not DataModelString")
        PackageSource(Path(dmstr.getValue()))
    }
}
extend PackageSource <: SerializableIncData<PackageSource> {
    public func incHash(): ?Hash {
        if (exists(dirpath) && FileInfo(canonicalize(dirpath)).isDirectory()) {
            return Directory.readFrom(dirpath).iterator()
                .filter { it => it.isRegular() &&
                    it.name.endsWith(".cj") &&
                    !it.name.endsWith("_test.cj")
                }
                .map { it => it.lastModificationTime } |>
                unorderedHash
        }
        return None
    }
}

private struct CompilerOptions <: Serializable<CompilerOptions> {
    CompilerOptions(
        let optionsHash: Hash
    ) {}
    public func serialize(): DataModel {
        optionsHash.serialize()
    }

    public static func deserialize(dm: DataModel): CompilerOptions {
        let dmint = dm as DataModelInt ?? throw DataModelException("this data is not DataModelInt")
        CompilerOptions(dmint.getValue())
    }

    static func fromString(options: String): CompilerOptions {
        CompilerOptions(options.hashCode())
    }
}

extend CompilerOptions <: SerializableIncData<CompilerOptions> {
    public func incHash(): ?Hash {
        return optionsHash
    }
}

private enum ArtifactCheckKind <: ToString & Serializable<ArtifactCheckKind> {
    | CheckExists
    | CheckTimestamp

    public func toString(): String {
        match (this) {
            case CheckExists => "check-exists"
            case CheckTimestamp => "check-timestamp"
        }
    }

    public func serialize(): DataModel {
        DataModelString(this.toString())
    }

    static func fromString(value: String): ArtifactCheckKind {
        match (value) {
            case "check-exists" => CheckExists
            case "check-timestamp" => CheckTimestamp
            case _ => throw Exception("Unknown kind of aftifact checking: `${value}`")
        }
    }

    public static func deserialize(dm: DataModel): ArtifactCheckKind {
        let dmstr = match (dm) {
            case dmstr: DataModelString => dmstr
            case _ => throw DataModelException("this is not DataModelString")
        }
        fromString(dmstr.getValue())
    }
}

@Derive[ToString]
private struct Artifact <: Serializable<Artifact> {
    private static let PATH_FIELD = "path"
    private static let CHECK_KIND_FIELD = "checkKind"
    Artifact(
        let path: String,
        let checkKind: ArtifactCheckKind
    ) { }

    public func serialize(): DataModel {
        DataModelStruct()
            .add(field(PATH_FIELD, path))
            .add(field(CHECK_KIND_FIELD, checkKind))
    }

    public static func deserialize(dm: DataModel): Artifact {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw DataModelException("this data is not DataModelStruct")
        }
        let path = String.deserialize(dms.get(PATH_FIELD))
        let checkKind = ArtifactCheckKind.deserialize(dms.get(CHECK_KIND_FIELD))
        Artifact(path, checkKind)
    }
}

extend Artifact <: SerializableIncData<Artifact> {
    public func incHash(): ?Hash {
        match (checkKind) {
            case CheckExists =>
                if (!exists(path)) { return None } else { return 0 }
            case CheckTimestamp =>
                if (!exists(path)) {
                    return None
                }
                try {
                    FileInfo(canonicalize(path)).lastModificationTime.hashCode()
                } catch (_: Exception) {
                    return None
                }
        }
    }
}

@Derive[ToString]
private struct ArtifactGroup <: Serializable<ArtifactGroup> {
    ArtifactGroup(
        let artifacts: Array<Artifact>
    ) { }

    public func serialize(): DataModel {
        artifacts.serialize()
    }

    public static func deserialize(dm: DataModel): ArtifactGroup {
        let dmseq = dm as DataModelSeq ?? throw DataModelException("this data is not DataModelSeq")
        ArtifactGroup(Array<Artifact>.deserialize(dmseq))
    }
}

extend ArtifactGroup <: SerializableIncData<ArtifactGroup> {
    public func incHash(): ?Hash {
        let size = artifacts.size
        let checked = artifacts.filterMap { it => it.incHash() } |> collectArray

        if (checked.size < size) {
            return None
        }

        checked |> unorderedHash
    }
}

@Derive[Hashable, Equatable]
public enum TaskId <: Serializable<TaskId> & ToString {
    // fullname, sourceSetDir
    | ReadPackageSource(String, String)
    // fullname, target, debug, mock, sourceSetDir
    | ReadCompilerOptions(String, String, Bool, Bool, String)
    // fullName, target, debug, mock, outputType, compilePhase, sourceSetDir
    | Compile(String, String, Bool, Bool, OutputType, CompilePhase, String)
    | BuildScriptCompile
    | Generated(UInt64)
    | Unknown(String)

    public func toString(): String {
        match (this) {
            case ReadPackageSource(fullName, sourceSetDir) => "read~${fullName}:${sourceSetDir}"
            case ReadCompilerOptions(fullName, target, debug, mock, sourceSetDir) => "options~${fullName}:${target}:${debug}:${mock}:${sourceSetDir}"
            case Compile(fullName, target, debug, mock, outputType, compilePhase, sourceSetDir) => "compile~${fullName}:${target}:${debug}:${mock}:${outputType}:${compilePhase}:${sourceSetDir}"
            case BuildScriptCompile => "build-script~"
            case Generated(i) => "${i}~"
            case Unknown(s) => s
        }
    }

    public func serialize(): DataModel {
        match (this) {
            case Generated(_) => throw Exception("TODO: remove")
            case _ => DataModelString(this.toString())
        }
    }

    static func fromString(value: String): TaskId {
        let splitted = value.split("~", 2)
        let values = splitted[1].split(":")
        match (splitted[0]) {
            case "read" => TaskId.ReadPackageSource(values[0], values.get(1) ?? "")
            case "options" =>
                TaskId.ReadCompilerOptions(
                    values[0],
                    values[1],
                    Bool.parse(values[2]),
                    Bool.parse(values[3]),
                    values.get(4) ?? "")
            case "compile" =>
                TaskId.Compile(
                    values[0],
                    values[1],
                    Bool.parse(values[2]),
                    Bool.parse(values[3]),
                    OutputType.fromString(values[4]),
                    CompilePhase.fromString(values[5]),
                    values.get(6) ?? ""
                )
            case "build-script" => TaskId.BuildScriptCompile
            case _ => Unknown(value)
         }
    }

    public static func deserialize(dm: DataModel): TaskId {
        let dmstr = match (dm) {
            case dmstr: DataModelString => dmstr
            case _ => throw DataModelException("this data is not DataModelString")
        }
        fromString(dmstr.getValue())
    }
}

// TASKS
class TaskContext {
    private var finalizedChecker: () -> Unit = { => }
    private var idCounter: UInt64 = 0

    public TaskContext(
        let taskGraphBuilder!: DAGBuilder<TaskId, Unit>,
        let taskStorage!: HashMap<TaskId, ErasedTask>,
        private let cache!: IncrementalCache
    ) { }

    public func setFinalizedChecker(checkFn: () -> Unit): Unit {
        this.finalizedChecker = checkFn
    }

    public func freshId(): TaskId {
        let result = idCounter
        idCounter += 1
        return TaskId.Generated(result)
    }

    public func checkFinalized(): Unit {
        finalizedChecker()
    }

    func createTask<R>(id!: TaskId, dependencies!: Array<TaskId>, comp!: () -> R, cache!: ?IncrementalCache = None): Task<R>
        where R <: SerializableIncData<R>
    {
        checkFinalized()
        taskGraphBuilder.add(id, incoming: dependencies, data: ())
        let out = Task(id, cache, this, comp)
        taskStorage.add(id, out)
        out
    }
}

sealed abstract class ErasedTask {
    var _value: Option<IncData> = None
    var logFiles: ?(String, String) = None

    ErasedTask(
        let id: TaskId,
        let cache: ?IncrementalCache
    ) { }

    public prop isPersistent: Bool {
        get() {
            cache.isSome()
        }
    }

    protected func compute(): Unit
    protected func cleanCompute(): Unit
    protected func asDataModel(): DataModel
    protected func tryComputedHash(): ?Hash
    protected func computedHash(): Hash
    protected func readValidCachedValue(): Bool
}

class Task<T> <: ErasedTask where T <: SerializableIncData<T> {
    private let computation: Option<() -> T>
    private let context: TaskContext

    init(id: TaskId, cache: ?IncrementalCache, context: TaskContext, comp: () -> T) {
        super(id, cache)
        this.computation = comp
        this.context = context
    }

    mut prop value: Option<T> {
        get() {
            if (let Some(incData) <- _value) {
                match (incData) {
                    case v: T => v
                    case _ => throw DataMistypeException(id)
                }
            } else {
                Option<T>.None
            }
        }
        set(newval) {
            if (let Some(v) <- newval) {
                this._value = v
            } else { this._value = None }
        }
    }

    protected func tryComputedHash(): ?Hash {
        if (let Some(res) <- value) {
            let hash = try { res.incHash() } catch (_: Exception) { throw NotComputedInputException(id) }
            return hash
        } else { throw NotComputedInputException(id) }
    }

    protected func computedHash(): Hash {
        tryComputedHash() ?? throw MissingArtifactException(id)
    }

    protected func asDataModel(): DataModel {
        if (let Some(res) <- value) {
            let dm = try { res.serialize() } catch (_: Exception) { throw NotComputedInputException(id) }
            return dm
        } else { throw NotComputedInputException(id) }
    }

    protected func compute(): Unit {
        if (this.value.isNone()) {
            cleanCompute()
        }
    }

    protected func cleanCompute(): Unit {
        if (let Some(comp) <- computation) {
            this.value = try {
                comp()
            } catch (e: Exception) {
                throw TaskComputeException(id, e)
            }
        }
    }

    protected func readValidCachedValue(): Bool {
        if (let Some(cache) <- cache) {
            if (let Some(dm) <- cache.readValue(id)) {
                try {
                    let result = T.deserialize(dm)
                    if (let Some(storedHash) <- cache.readOutputHash(id) && result.incHash() == storedHash) {
                        this.value = result
                        return true
                    }
                } catch(e: Exception) { }
            }
        }
        return false
    }

    public func map<U>(transform: (T) -> U): Task<U> where U <: SerializableIncData<U> {
        context.createTask(
            id: context.freshId(),
            dependencies: [id],
            comp: { => transform(value ?? throw NotComputedInputException(id)) },
            cache: None
        )
    }
}

class EngineConfig {
    public EngineConfig(
        public let incremental!: Bool = true,
        public let cutoff!: Bool = true,
        public let jobs!: ?Int64 = None
    ) { }
}

// INCREMENTAL ENGINE
class IncrementalEngine {
    let taskGraphBuilder = DAG<TaskId, Unit>.builder()
    let taskStorage = HashMap<TaskId, ErasedTask>()
    let dirtyTasks = HashSet<TaskId>()
    let cache: IncrementalCache
    let taskContext: TaskContext
    let config: EngineConfig

    private var finalized = false
    private var askAllFlag = false
    private let askedTasks = ArrayList<TaskId>()

    init(
        cacheLocation!: Path,
        config!: EngineConfig = EngineConfig()
    ) {
        let storage = taskStorage
        this.config = config
        this.cache = IncrementalCache(
            cacheLocation: cacheLocation,
            isPersistent: { id => storage.get(id)?.isPersistent ?? false }
        )
        this.taskContext = TaskContext(
            taskGraphBuilder: taskGraphBuilder,
            taskStorage: taskStorage,
            cache: cache
        )
        this.taskContext.setFinalizedChecker { 
            if (this.finalized) {
                throw FinalizedBuildException()
            }
        }
    }

    func askAll(): Unit {
        taskContext.checkFinalized()
        askAllFlag = true
    }

    func ask(taskId: TaskId): Unit {
        taskContext.checkFinalized()
        askedTasks.add(taskId)
    }

    private func processAskedTasks() {
        let graph = if (askAllFlag) {
            taskGraphBuilder.finish()
        } else {
            let ids = askedTasks.toArray()
            taskGraphBuilder.finish().incomingClosure(ids)
        }

        for (id in graph.roots where !dirtyTasks.contains(id)) {
            let input = taskStorage.get(id) ?? throw TaskNotFoundException(id)
            input.compute()
            if (let newHash <- input.computedHash() && cache.readOutputHash(id) != newHash) {
                cache.update(id, 0, newHash, input.asDataModel())
            }
        }

        for (id in graph.toposort where !dirtyTasks.contains(id)) {
            var isClean = true
            let dependencies = graph.incoming(id)
            isClean &&= dependencies |> all { it => !dirtyTasks.contains(it) }
            isClean &&= cache.computeDependenciesHash(dependencies) == cache.readInputHash(id)
            isClean &&= taskStorage[id].readValidCachedValue()

            if (!isClean) {
                dirtyTasks.add(id)
            }
        }

    }

    // `dirtyGraph` - a subgraph of `graph` of tasks to be executed
    // `graph` - a whole graph of tasks. Provided for proper incrementality and cache updates
    private func executeSubgraph(dirtyGraph: DAG<TaskId, Unit>, graph: DAG<TaskId, Unit>): TaskResult {
        taskContext.checkFinalized()

        let rebuilder = Rebuilder(
            taskStorage: taskStorage,
            dirtyGraph: dirtyGraph,
            taskGraph: graph,
            cache: cache,
            cutoff: config.cutoff,
            incremental: config.incremental,
            jobs: config.jobs
        )

        let result = rebuilder.rebuild()

        cache.storeCache()

        finalized = true
        return result
    }

    func execute(): TaskResult {
        processAskedTasks()
        let graph = taskGraphBuilder.finish()
        let dirtyGraph = graph.filterGraph { it: TaskId, _ => dirtyTasks.contains(it) }
        executeSubgraph(dirtyGraph, graph)
    }

    func executeClean(): TaskResult {
        let graph = taskGraphBuilder.finish()
        executeSubgraph(graph, graph)
    }
}

// Returns an array of compiler calls, which are required to compile a package
func createPackageCompileTasks(r: ResolveItem, buildConfig: BuildConfig, productOutputTypes: HashMap<String, OutputType>): ArrayList<CompileTask> {
    // Common to all tasks, but needs to be embedded in CompileTask struct
    let isDebug = buildConfig.isDebug
    let isCov = buildConfig.isCov
    let requiredForTests = buildConfig.requiredForTests
    let mockSupported = buildConfig.mockSupported

    // Common to all tasks, needed to compile the package
    let fullName = r.fullName
    let rootPkgName = r.rootPkgName
    let isMultiplatform = r.packagePath.isMultiplatform
    let customizedOption = constructCustomizedOption(r, buildConfig)
    let hasSubPkgs = buildConfig.hasSubPkgs.contains(fullName)
    let exportForTests = requiredForTests && r.hasTestFiles
    let isMacro = buildConfig.packageList.isMacro(fullName)
    let isAnalysisCompilePerformance = !isMultiplatform && buildConfig.globalConfig.isAnalysisCompilePerformance

    let allEnabledFeatures = r.featureDeducer
        .addFeature(r.features)
        .collect()
    let featureMapping = r.featureDeducer.cleanFeatures() // leaving only mapping to apply them to source set features

    let productSuffixes = buildConfig.packageList.productSuffixes

    // Specific to the `--target` platform
    // `isNativeForCross == true` - package needs to be compiled with `host` target to be used by or is a `macro package` 
    func createForTarget(isNativeForCross!: Bool = false): ArrayList<CompileTask> {
        let targetDir = if (isNativeForCross) { buildConfig.globalConfig.nativeDir } else { buildConfig.globalConfig.targetDir }

        let isCrossCompile = buildConfig.isCrossCompile && !isNativeForCross

        let target = if (isCrossCompile) {
            crossCompileTarget
        } else { targetConfigName }

        let compileOption = if (isNativeForCross) {
            r.nativeCompileOption
        } else { r.compileOption }

        let overrideOption = if (isNativeForCross) {
            buildConfig.globalConfig.nativeOverrideOption
        } else { buildConfig.globalConfig.overrideCompileOption }

        let linkOption = if (isNativeForCross) {
            r.nativeLinkOption
        } else { r.linkOption }

        let sourceSetDAG = r.packagePath.sourceSets
        let srcDirToTaskId = HashMap<String, TaskId>()
        let srcDirToTargetPath = HashMap<String, String>()

        // Multiplatform packages need an arbitrary number of `cjc` calls
        // Non-multiplatform packages consist of single `sourceSetEntry`
        func createForSingleSourceSet(sourceSetEntry: CJMPPackageInfo): ArrayList<CompileTask> {
            let packagePath = sourceSetEntry.srcDir.toString()
            let product = sourceSetEntry.product
            let sourceSetDir = sourceSetEntry.outputSuffix.toString()
            let sourceSetFeatures = sourceSetEntry.features

            let subPkgSet = r.superPkgCfg?.subPkgSet ?? HashSet<String>()
            let isCombined = product && r.superPkgCfg.isSome() && !isNativeForCross
            let isProjectCombined =
                product &&
                buildConfig.globalConfig.isProjectCombine &&
                r.fullName == buildConfig.globalConfig.rootName &&
                !isNativeForCross

            let supressed = ArrayList<String>()

            let outputType = if (!product) {
                OutputType.Chir
            } else if (r.outputType == Exe && requiredForTests) {
                // to be included in tests as a dependency, the package should be compiled as a library anyway
                // unused main function related warnings should also be suppressed in this scenario
                supressed.add("-Woff=unused-main")
                OutputType.Static
            } else { r.outputType }

            let isLto = buildConfig.isLto && (outputType == Static || outputType == Exe)
            let ltoValue = if (isLto) { buildConfig.ltoValue } else { "" }

            let isSplittable = !isMultiplatform &&
                !isLto &&
                product &&
                buildConfig.isCompilePipelineParallel &&
                !isMacro &&
                (outputType == Dynamic || outputType == Exe)
 
            let compilePhase = if (isSplittable) { ObjOnly } else { CompilePhase.Default }

            let taskId = TaskId.Compile(fullName, target, isDebug, mockSupported, outputType, compilePhase, sourceSetDir)

            let filename = if (outputType == Exe && buildConfig.packageList.exe.size <= 1 && !COMMON_INFO.inWorkspace) {
                buildConfig.exeName
            } else { fullName }

            let targetPath = if (outputType == Exe) {
                Path(targetDir).join(BIN).join(sourceSetDir).toString()
            } else {
                Path(targetDir).join(swapOrgName(rootPkgName)).join(sourceSetDir).toString()
            }
            let logPath = Path(targetDir).join(".build-logs").join(swapOrgName(rootPkgName)).join(sourceSetDir).toString()

            let requireTasksIterator = 
                if (buildConfig.globalConfig.isProjectCombine && fullName == buildConfig.globalConfig.rootName) {
                    productOutputTypes.keys().iterator()
                } else {
                    var result = r.requires.iterator()
                    result = result |> concat(subPkgSet)
                    result
                }

            // Requires is not only the compilation tasks. It's the names of all dependencies
            // We need to leave only packages we're creating compilation tasks for
            let requireTasks = requireTasksIterator
                .filter { it => productOutputTypes.contains(it) }
                .map { name =>
                    let isMacroName = buildConfig.packageList.isMacro(name)
                    let waitTarget = if (isMacroName) { 
                        targetConfigName
                    } else { target }
                    let isSplittable = !isMultiplatform &&
                        !isLto &&
                        buildConfig.isCompilePipelineParallel &&
                        !isMacroName &&
                        (buildConfig.packageList.isDyLib(name) || buildConfig.packageList.isExe(name))
                    let compilePhase = if (isSplittable) { ObjOnly } else { CompilePhase.Default }
                    TaskId.Compile(
                        name, waitTarget, isDebug, mockSupported,
                        productOutputTypes[name], compilePhase, productSuffixes.get(name) ?? ""
                    )
                } |> collectHashSet
            requireTasks.add(TaskId.ReadPackageSource(fullName, sourceSetDir))
            requireTasks.add(TaskId.ReadCompilerOptions(fullName, target, isDebug, mockSupported, sourceSetDir))

            let (outLogFile, errLogFile) = getLogFile(logPath, taskId)

            srcDirToTaskId.add(sourceSetEntry.sourceSetDir.toString(), taskId)
            srcDirToTargetPath.add(sourceSetEntry.sourceSetDir.toString(), targetPath)

            let previousSourceSets = sourceSetDAG.incoming(sourceSetEntry.sourceSetDir.toString())
                .map { it => srcDirToTaskId[it] }
            requireTasks.add(all: previousSourceSets)
            let prevStageArtifacts: Array<(String, String)> = sourceSetDAG.incoming(sourceSetEntry.sourceSetDir.toString())
                .map { it => srcDirToTargetPath[it] }
                .map { it => (Path(it).join("${fullName}.chir").toString(), Path(it).join("${fullName}.cjo").toString()) }

            let task = CompileTask(
                id: taskId,
                targetDir: targetDir,
                targetPath: targetPath,
                packagePath: packagePath,
                product: product,
                sourceSetDir: sourceSetDir,
                sourceSetFeatures: sourceSetFeatures,
                rootPkgName: rootPkgName,
                fullName: fullName,
                filename: filename,
                outputType: if (isSplittable) { Obj } else { outputType },
                target: target,
                isCrossCompile: isCrossCompile,
                compileOption: compileOption,
                overrideOption: overrideOption,
                linkOption: linkOption,
                customizedOption: customizedOption,
                subPkgSet: subPkgSet,
                isCombined: isCombined,
                isProjectCombined: isProjectCombined,
                requiredForTests: requiredForTests,
                exportForTests: exportForTests,
                requireTasks: requireTasks,
                allEnabledFeatures: allEnabledFeatures,
                featureMapping: featureMapping,
                isAnalysisCompilePerformance: isAnalysisCompilePerformance,
                hasSubPkgs: hasSubPkgs,
                isDebug: isDebug,
                isCov: isCov,
                mockSupported: mockSupported,
                isMacro: isMacro,
                isLto: isLto,
                ltoValue: ltoValue,
                prevStageArtifacts: prevStageArtifacts,
                isMultiplatform: isMultiplatform,
                supressed: supressed,
                logPath: logPath,
                outLogFile: outLogFile,
                errLogFile: errLogFile,
                compileTargetOutput: if (isSplittable) { outputType } else { None },
                compilePhase: compilePhase
            )

            let compileTasks = ArrayList<CompileTask>([task])
 	 
            if (isSplittable) {
                let taskId = TaskId.Compile(fullName, target, isDebug, mockSupported, outputType, LinkOnly, sourceSetDir)
                let prevStageObj = Path(task.targetPath).join(getOutput(task).getOrThrow()).toString()
                let requireTasks = getDepSet(fullName, buildConfig).iterator().filterMap { name: String =>
                        if(!buildConfig.packageList.isMacro(name) && !buildConfig.packageList.isTest(name)) {
                            let isSplittable = buildConfig.packageList.isDyLib(name) || buildConfig.packageList.isExe(name)
                            let compilePhase = if (isSplittable) { LinkOnly } else { CompilePhase.Default }
                            TaskId.Compile(name, target, isDebug, mockSupported, productOutputTypes[name], compilePhase, productSuffixes.get(name) ?? "")
                        } else {
                            None
                        }
                    } |> collectHashSet
                requireTasks.add(task.id)
                let (outLogFile, errLogFile) = getLogFile(logPath, taskId)
                let linkTask = CompileTask(
                    id: taskId,
                    targetDir: targetDir,
                    targetPath: targetPath,
                    packagePath: packagePath,
                    product: product,
                    sourceSetDir: sourceSetDir,
                    sourceSetFeatures: sourceSetFeatures,
                    rootPkgName: rootPkgName,
                    fullName: fullName,
                    filename: filename,
                    outputType: outputType,
                    target: target,
                    isCrossCompile: isCrossCompile,
                    compileOption: compileOption,
                    overrideOption: overrideOption,
                    linkOption: linkOption,
                    customizedOption: customizedOption,
                    subPkgSet: subPkgSet,
                    isCombined: isCombined,
                    isProjectCombined: isProjectCombined,
                    requiredForTests: requiredForTests,
                    exportForTests: exportForTests,
                    requireTasks: requireTasks,
                    allEnabledFeatures: allEnabledFeatures,
                    featureMapping: featureMapping,
                    isAnalysisCompilePerformance: isAnalysisCompilePerformance,
                    hasSubPkgs: hasSubPkgs,
                    isDebug: isDebug,
                    isCov: isCov,
                    mockSupported: mockSupported,
                    isMacro: isMacro,
                    isLto: isLto,
                    ltoValue: ltoValue,
                    isMultiplatform: isMultiplatform,
                    supressed: supressed,
                    logPath: logPath,
                    outLogFile: outLogFile,
                    errLogFile: errLogFile,
                    prevStageObj: prevStageObj,
                    compilePhase: LinkOnly
                )
                compileTasks.add(linkTask)
            }
            compileTasks
        }

        let sourceSetUnwrap = ArrayList<CompileTask>()
        for (sourceSetEntry in r.packagePath._sources) {
            let tasks = createForSingleSourceSet(sourceSetEntry)
            sourceSetUnwrap.add(all: tasks)
        }
        sourceSetUnwrap
    }

    let result = ArrayList<CompileTask>()

    assertion { r.targetPlatform || r.nativePlatform }
    if (r.nativePlatform) {
        result.add(all: createForTarget(isNativeForCross: true))
    }
    if (r.targetPlatform) {
        result.add(all: createForTarget(isNativeForCross: false))
    }
    return result
}

private func constructCustomizedOption(r: ResolveItem, buildConfig: BuildConfig): ArrayList<String> {
    let customizedOption = ArrayList<String>()
    for (k in buildConfig.customizedOption) {
        if (let Some(option) <- r.customizedOption.get(k)) {
            customizedOption.add(option)
        }
    }
    return customizedOption
}

// REBUILDER
@Derive[Equatable, ToString]
enum TaskResult {
    | Cached
    | Rebuild
    | Fatal

    operator func +(b: TaskResult): TaskResult {
        match((this, b)) {
            case (Fatal, _) => Fatal
            case (_, Fatal) => Fatal
            case (Rebuild, _) => Rebuild
            case (_, Rebuild) => Rebuild
            case (Cached, Cached) => Cached
        }
    }
}

extend Array<TaskResult> {
    prop cached: Bool {
        get() {
            this.iterator().all { it => it == Cached }
        }
    }

    prop fatal: Bool {
        get() {
            this.iterator().any { it => it == Fatal }
        }
    }
}

class Rebuilder {
    private let futureMap: HashMap<TaskId, Future<TaskResult>>
    private let logger: BoundedFileLogger

    Rebuilder(
        private let taskStorage!: ReadOnlyMap<TaskId, ErasedTask>,
        private let dirtyGraph!: DAG<TaskId, Unit>,
        private let taskGraph!: DAG<TaskId, Unit>,
        private let cache!: IncrementalCache,
        private let cutoff!: Bool,
        private let incremental!: Bool,
        private let jobs!: ?Int64
    ) {
        let tasksCount = dirtyGraph.toposort.size
        this.futureMap = HashMap<TaskId, Future<TaskResult>>(tasksCount)
        this.logger = BoundedFileLogger(tasksCount)
    }

    func rebuild(): TaskResult {
        logger.start()

        let result = match (jobs) {
            case None => rebuildUnlimited()
            case Some(maxJobs) => rebuildLimited(maxJobs)
        }

        if (!logger.unsafeWait()) {
            Fatal
        } else { result }
    }

    private func rebuildUnlimited(): TaskResult {
        /*
         * To avoid using `Mutex` to populate `futureMap` 
         * we will make a barrier with `taskCount` + 1
         * So each `Future` will wait for `futureMap` to be full
         */
        let barrier = Barrier(dirtyGraph.toposort.size + 1)
        for (id in dirtyGraph.toposort) {
            let task = taskStorage[id]
            let future = spawn { =>
                barrier.wait()
                deleteLog(task.logFiles)

                let depsResult = waitForDependencies(id)
                let result = executeTask(task, depsResult)

                logger.log(task.logFiles)
                result
            }
            futureMap.add(id, future)
        }

        barrier.wait()
        return collectResults()
    }

    private func rebuildLimited(maxJobs: Int64): TaskResult {
        /*
         * Same barrier as unlimited case to populate `futureMap` first.
         * Then wait for dependencies before acquiring semaphore to ensure
         * topological ordering of semaphore acquisition.
         */
        let semaphore = Semaphore(maxJobs)
        let barrier = Barrier(dirtyGraph.toposort.size + 1)

        for (id in dirtyGraph.toposort) {
            let task = taskStorage[id]
            let future = spawn { =>
                barrier.wait()

                let depsResult = waitForDependencies(id)

                semaphore.acquire()
                let result = executeTask(task, depsResult)
                semaphore.release()

                result
            }
            futureMap.add(id, future)
        }

        barrier.wait()
        return collectResults()
    }

    private func waitForDependencies(id: TaskId): Array<TaskResult> {
        dirtyGraph.incoming(id).map { it: TaskId => futureMap[it].get() }
    }

    private func collectResults(): TaskResult {
        var result = Cached
        for (future in futureMap.values()) {
            result += future.get()
        }
        return result
    }

    private func executeTask(task: ErasedTask, dependenciesResult: Array<TaskResult>): TaskResult {
        let id = task.id

        if (dependenciesResult.fatal) {
            return Fatal
        }

        let dependencies = taskGraph.incoming(id)

        try {
            if (incremental && cutoff && dependenciesResult.cached) {
                if (canCutoffEarly(id, dependencies)) {
                    return Cached
                }
            }

            task.compute()

            /*
             * Safeguard: if a task succeeds but its output artifact differs from expected,
             * we fail caching instead of incorrectly marking the task as failed.
             */
            let computedHash = if (let Some(computedHash) <- task.tryComputedHash()) {
                computedHash
            } else {
                return Rebuild
            }

            let newInputCache = dependencies |> filterMap { it: TaskId => cache.readOutputHash(it) } |> unorderedHash
            cache.update(id, newInputCache, computedHash, task.asDataModel())

            if (incremental && cutoff && let Some(hash) <- cache.readOutputHash(id) && computedHash == hash) {
                return Cached
            }
            return Rebuild
        } catch (e: IncrementalEngineException) {
            return Fatal
        }
    }

    private func canCutoffEarly(id: TaskId, dependencies: Array<TaskId>): Bool {
        match ((cache.computeDependenciesHash(dependencies), cache.readInputHash(id))) {
            case (Some(depsHash), Some(inputHash)) => depsHash == inputHash && taskStorage[id].readValidCachedValue()
            case _ => false
        }
    }
}

// CACHING

// Non-cacheable data (e.g. root inputs, always computed)
interface IncData {
    // case None  - Although, value is known, the current state is invalid and the task should be recomputed
    // case value - The value is valid, and can be reused
    func incHash(): ?Hash
}

// Cacheable data
interface SerializableIncData<T> <: IncData where T <: Serializable<T> {
    // case None  - Although, value is known, the current state is invalid and the task should be recomputed
    // case value - The value is valid, and can be reused
    func incHash(): ?Hash
}

private class CacheItem <: Serializable<CacheItem> {
    private static let VALUE = "value"
    private static let INPUT_HASH = "inputHash"
    private static let OUTPUT_HASH = "outputHash"

    CacheItem(
        let value: DataModel,
        let inputHash: Hash,
        let outputHash: Hash
    ) { }

    public func serialize(): DataModel {
        DataModelStruct()
            .add(Field(VALUE, value))
            .add(field(INPUT_HASH, inputHash))
            .add(field(OUTPUT_HASH, outputHash))
    }

    public static func deserialize(dm: DataModel): CacheItem {
        let dms = match (dm) {
            case dms: DataModelStruct => dms
            case _ => throw DataModelException("this data is not DataModelStruct")
        }
        let value = dms.get(VALUE)
        let inputHash = Hash.deserialize(dms.get(INPUT_HASH))
        let outputHash = Hash.deserialize(dms.get(OUTPUT_HASH))
        CacheItem(value, inputHash, outputHash)
    }
}

class IncrementalCache {
    private let inMemoryCache = ConcurrentHashMap<TaskId, Hash>()
    private let persistentCache: ConcurrentHashMap<TaskId, CacheItem>

    IncrementalCache (
        private let cacheLocation!: Path,
        private let isPersistent!: (TaskId) -> Bool
    ) {
        this.persistentCache = readCacheFromJson(cacheLocation)
    }

    func update(id: TaskId, inputHash: Hash, outputHash: Hash, dm: DataModel): Unit {
        if (isPersistent(id)) {
            persistentCache.add(id, CacheItem(dm, inputHash, outputHash))
        } else {
            inMemoryCache.add(id, outputHash)
        }
    }

    func readValue(id: TaskId): ?DataModel {
        if (isPersistent(id)) {
            persistentCache.get(id)?.value
        } else {
            None
        }
    }

    func readInputHash(id: TaskId): ?Hash {
        if (isPersistent(id)) {
            persistentCache.get(id)?.inputHash
        } else {
            None
        }
    }

    func readOutputHash(id: TaskId): ?Hash {
        if (isPersistent(id)) {
            persistentCache.get(id)?.outputHash
        } else {
            inMemoryCache.get(id)
        }
    }

    func computeDependenciesHash(dependencies: Array<TaskId>): ?Hash {
        let cachedDeps = dependencies |> filterMap { it => readOutputHash(it) } |> collectArray
        if (dependencies.size != cachedDeps.size) {
            return None
        }
        Some(cachedDeps.iterator() |> unorderedHash)
    }


    private static func readCacheFromJson(cacheLocation: Path): ConcurrentHashMap<TaskId, CacheItem> {
        let dm = readJsonFileOrEmpty(cacheLocation)
        try {
            // stdx.serialization.serialization doesn't work properly with non-string keys
            let hashmap = HashMap<String, CacheItem>.deserialize(dm)
                .iterator().map {it => (TaskId.fromString(it[0]), it[1])} |>
                collectHashMap
            ConcurrentHashMap(hashmap)
        } catch (e: Exception) {
            ConcurrentHashMap()
        }
    }

    func storeCache(): Unit {
        writeJsonToFile(cacheLocation, persistentCache)
    }
}

private func readJsonFileOrEmpty(path: Path): DataModel {
    try {
        let barray = File.readFrom(path)
        let s = String.fromUtf8(barray)
        let json = JsonValue.fromStr(s)
        let res = DataModel.fromJson(json)
        return res
    } catch (e: Exception) {
        DataModel.fromJson(JsonObject())
    }
}

private func writeJsonToFile(path: Path, cache: ConcurrentHashMap<TaskId, CacheItem>): Unit {
    try {
        // stdx.serialization.serialization doesn't work properly with non-string keys
        let dm = (cache |> map { it => (it[0].toString(), it[1]) } |> collectHashMap).serialize()
        let json = dm.toJson()
        let s = json.toJsonString()
        let barray = s.toArray()
        File.writeTo(path, barray)
    } catch (e: Exception) { }
}


private func callCompiler(compileTask: CompileTask, args: ArrayList<String>, envBuilder: EnvironmentBuilder, buildConfig: BuildConfig): Bool {
    var execCmdFlag: Bool = true

    CUR_PARALLEL_SIZE.fetchAdd(1)
    if (!futJudge(args)) {
        let jobs = calculateParallel(CUR_PARALLEL_SIZE.load())
        args.add("-j${jobs}", at: 0)
    }

    let env = getVariables()
    let commandStr = getCmdStr(envBuilder.asCliStrings(env), COMPILE_TOOL, args)
    if (buildConfig.isVerbose) {
        let verbose: String = if (compileTask.isMultiplatform) {
            "Compiling `${compileTask.sourceSetDir}` part of package `${compileTask.fullName}`: ${commandStr}\n"
        } else { 
            "Compiling package `${compileTask.fullName}`: ${commandStr}\n"
        }
        writeFile(compileTask.outLogFile, verbose, Append)
    }

    if (compileTask.isAnalysisCompilePerformance) {
        addStartTime(compileTask.fullName, commandStr)
    }

    let outFilePath = compileTask.outLogFile
    let errFilePath = compileTask.errLogFile
    let (outFile, errFile) = try {
        (File(outFilePath, OpenMode.Append), File(errFilePath, OpenMode.Append))
    } catch (e: Exception) {
        eprintln(e.message)
        eprintln("Error: create '${outFilePath}' failed")
        return false
    }

    try {
        if (let Some(returnCode) <- execAndToFile(COMPILE_TOOL, args, outFile, errFile, envBuilder: envBuilder, originalEnv: env)) {
            execCmdFlag = (returnCode == 0)
            if (execCmdFlag) {
                if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                    moveCjcProfToCjpmDir(compileTask, buildConfig.globalConfig.compilePerformanceTargetDir)
                }
            } else {
                let errmesg = if (compileTask.isMultiplatform) {
                    "Error: failed to compile `${compileTask.sourceSetDir}` part of package `${compileTask.fullName}`, return code is ${returnCode}\n"
                } else {
                    "Error: failed to compile package `${compileTask.fullName}`, return code is ${returnCode}\n"
                }
                errFile.write(errmesg.toArray())
            }
        } else {
            errFile.write("Error: failed to compile package `${compileTask.fullName}` with exception occurred\n".toArray())
        }
    } catch (e: Exception) {
        eprintln("Error: failed to write error log into ${errFilePath}: ${e.message}")
        execCmdFlag = false
    }

    CUR_PARALLEL_SIZE.fetchSub(1)
    outFile.close()
    errFile.close()

    if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
        addEndTime(compileTask.fullName)
    }
    return execCmdFlag
}

// COMPILER PROFILING
func moveCjcProfToCjpmDir(compileTask: CompileTask, profcompilePerformancePath: String): Unit {
    let profSuffixArray: Array<String> = [".mem.prof", ".time.prof", ".info.prof"]
    for (profSuffix in profSuffixArray) {
        let pkgNameProfPath = Path(compileTask.targetPath).join(compileTask.fullName + profSuffix)
        if (exists(pkgNameProfPath)) {
            let profPath = Path(profcompilePerformancePath).join(compileTask.fullName + profSuffix)
            copy(pkgNameProfPath, to: profPath, overwrite: true)
            remove(pkgNameProfPath)
        } else {
            eprintln("Error: cannot find ${pkgNameProfPath.toString()} file")
        }
    }
    return
}

func addStartTime(fullName: String, cmdArgs: String): Unit {
    synchronized(CACHE_MUTEX) {
        SHOW_CACHE.add(PkgInfo(fullName, "B", DateTime.now().toUnixTimeStamp().toMilliseconds()))
        COMMAND_CACHE.add(CommandInfo(fullName, cmdArgs))
    }
    return
}

func addEndTime(fullName: String): Unit {
    synchronized(CACHE_MUTEX) {
        SHOW_CACHE.add(PkgInfo(fullName, "E", DateTime.now().toUnixTimeStamp().toMilliseconds()))
    }
    return
}

// PARALLELIZATION
func printDependency(taskA: TaskId, taskB: TaskId, verbose: Bool): Unit {
    if (!verbose) {
        return
    }
    println("`${taskA}` depends on `${taskB}`")
}

// WHERE DID IT GO?
func specifyCjcJobs(args: ArrayList<String>, cjcParallelConfig: CjcParallelConfig): Unit {
    CUR_PARALLEL_SIZE.fetchAdd(1)

    if (futJudge(args)) {
        return
    }

    match (cjcParallelConfig) {
        case CjcParallelConfig.Default => ()
        case CjcParallelConfig.Heuristic =>
            let jobs = calculateParallel(CUR_PARALLEL_SIZE.load())
            args.add("-j${jobs}", at: 0)
        case CjcParallelConfig.Specific(v) =>
            args.add("-j${v}", at: 0)
    }
}

func calculateParallel(currentParalla: Int64): Int64 {
    return if (currentParalla <= 2) {
        maxParallelSize
    } else {
        2 * maxParallelSize / currentParalla
    }
}

func futJudge(args: ArrayList<String>): Bool {
    let tmp = args.toString()
    let fut1: Future<Bool> = spawn {
        let regexParallel = Regex("-j[[:space:]]*(?:[[:digit:]]|=[[:digit:]])")
        regexParallel.matches(tmp)
    }
    let fut2: Future<Bool> = spawn {
        tmp.contains("--jobs") || args.contains("-j")
    }
    return (fut1.get() || fut2.get())
}

func getLogFile(dir: String, taskId: TaskId): (String, String) {
    match (taskId) {
        case Compile(fullName, target, _, _, _, compilePhase, sourceSetDir) =>
            let logFileName = "${swapOrgName(fullName)}.${target}.${sourceSetDir}.${compilePhase}".replace("/", "#").replace("\\", "#")
            let outLogFile = Path(dir).join("${logFileName}.outlog").toString()
            let errLogFile = Path(dir).join("${logFileName}.errlog").toString()
            return (outLogFile, errLogFile)
        case _ => throw Exception("TODO")
    }
}

var watcherCb: (String) -> Unit = { _ => }

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
func watchFS(cjo2task: HashMap<String, TaskId>, listenDirs: HashSet<String>,
    notify: (TaskId, Notification) -> Unit, collectCjoFlagFiles: (String) -> Bool, verbose: Bool): Unit {
    let dirs = StringBuilder()
    for (dir in listenDirs) {
        if (!exists(dir)) {
            Directory.create(dir, recursive: true)
        }
        dirs.append("${dir},")
    }
    watcherCb = { cjo => notify(cjo2task[cjo], CjoReady) }
    unsafe {
        let cs = LibC.mallocCString(dirs.toString())
        let res = initFSWatcher(cs, { cjo => watcherCb(cjo.toString()) }, verbose)
        LibC.free(cs)
        if (!res) {
            return
        }
        startFSWatcher(verbose)
    }
}

@When[(os == "iOS") || (env == "ohos") || (env == "android")]
func watchFS(_: HashMap<String, TaskId>, _: HashSet<String>,
    _: (TaskId, Notification) -> Unit, _: (String) -> Bool, verbose: Bool): Unit {}

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
func stopWatchFS(isVerbose: Bool): Unit {
    unsafe { stopFSWatcher(isVerbose) }
}

@When[(os == "iOS") || (env == "ohos") || (env == "android")]
func stopWatchFS(_: Bool): Unit {}

// EXCEPTIONS
sealed abstract class IncrementalEngineException <: Exception {
    IncrementalEngineException(msg: String) { super(msg) }
}

class TaskNotFoundException <: IncrementalEngineException {
    TaskNotFoundException(id: TaskId) {
        super("Couldn't find task with id `${id}`")
    }
}

class AmbiguousTaskException <: IncrementalEngineException {
    AmbiguousTaskException(id: TaskId) {
        super("Task `${id}` is already defined")
    }
}

class FinalizedBuildException <: IncrementalEngineException {
    FinalizedBuildException() {
        super("IncrementalEngine object is finalized and cannot be changed")
    }
}

class DataMistypeException <: IncrementalEngineException {
    DataMistypeException(id: TaskId) {
        super("Task `${id}` stores the value of wrong type")
    }
}

class NotComputedInputException <: IncrementalEngineException {
    NotComputedInputException(id: TaskId) {
        super("Tried to get value from task `${id}` before calling `engine.execute()`")
    }
}

class MissingArtifactException <: IncrementalEngineException {
    MissingArtifactException(id: TaskId) {
        super("Task `${id}` was executed, but artifact didn't get produced or got missing")
    }
}

class TaskComputeException <: IncrementalEngineException {
    TaskComputeException(id: TaskId, error: Exception) {
        super("Task `${id}` threw an exception:\n${error}")
    }
}