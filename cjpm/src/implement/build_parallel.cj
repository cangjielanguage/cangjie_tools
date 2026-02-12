// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.collection.concurrent.*
import std.sync.*
import std.fs.*
import std.regex.*
import std.time.*
import std.env.getVariables

import cjpm.config.*
import cjpm.util.*

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
foreign func initFSWatcher(dirs: CString, cb: CFunc<(CString) -> Unit>, verbose: Bool): Bool

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
foreign func startFSWatcher(verbose: Bool): Unit

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
foreign func stopFSWatcher(verbose: Bool): Unit

type TaskMap = HashMap<TaskIdentifier, CompileTask>
type DownStreamTasks = HashMap<TaskIdentifier, HashSet<TaskIdentifier>>
type Task2SyncCounter = HashMap<TaskIdentifier, SyncCounter>

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

private enum MessageKind {
    | Release
    | Print(String, String)
}

// A very simple logger which can write at most `messageBound` messages read from files
public class BoundedFileLogger {
    private let messageQueue = LinkedBlockingQueue<MessageKind>()
    private var logFuture: ?Future<Bool> = None

    public BoundedFileLogger(
        private let messageBound: Int64
    ) { }

    public func release(): Unit {
        this.messageQueue.add(Release)
    }

    public func log(outLogFile: String, errLogFile: String): Unit {
        this.messageQueue.add(Print(outLogFile, errLogFile))
    }

    public func start(): This {
        if (logFuture.isNone()) {
            this.logFuture = spawn { =>
                var count = this.messageBound
                var success: Bool = true
                while (count > 0) {
                    match (this.messageQueue.remove()) {
                        case Release => count--
                        case Print(outLogFile, errLogFile) => 
                            count--
                            success = printLogs(outLogFile, errLogFile) && success
                    }
                }
                return success
            }
        }
        this
    }

    // If number of `log` and `release` calls is below the `messageBound`, deadlock occurs
    func unsafeWait(): Bool {
        if (let Some(logFuture) <- this.logFuture) {
            logFuture.get()
        } else { true }
    }


    private static func printLogs(outLogFile: String, errLogFile: String): Bool {
        var logFlag: Bool = true
        func printLogFile(logFile: String, toStrerr: Bool): Bool {
            if (!fileExists(logFile)) {
                return true
            }
            try {
                let logStr: String = unsafe { String.fromUtf8Unchecked(File.readFrom(logFile)) }
                if (toStrerr) {
                    eprint(logStr)
                } else {
                    print(logStr)
                }
            } catch (e: Exception) {
                eprintln(e.message)
                eprintln("Error: failed to read '${logFile}'")
                return false
            }
            return true
        }
        logFlag = printLogFile(outLogFile, false) && logFlag
        logFlag = printLogFile(errLogFile, true) && logFlag
        return logFlag
    }
}

class ParallelBuilder {
    let buildConfig: BuildConfig
    let taskMap: TaskMap
    let downstreamTasks: DownStreamTasks
    let scs: Task2SyncCounter
    let orderedTasks: Array<TaskIdentifier>
    let notifyMutex = Mutex()
    let notified = DownStreamTasks()
    let buildMutex = Mutex()
    let shouldFail = HashSet<TaskIdentifier>()

    init(buildConfig: BuildConfig, taskMap: TaskMap, downstreamTasks: DownStreamTasks, scs: Task2SyncCounter, orderedTasks: Array<TaskIdentifier>) {
        this.buildConfig = buildConfig
        this.taskMap = taskMap
        this.downstreamTasks = downstreamTasks
        this.scs = scs
        this.orderedTasks = orderedTasks
        orderedTasks.forEach { task => notified[task] = HashSet<TaskIdentifier>() }
    }

    func checkNoChange(depPkg: String): Bool {
        if (buildConfig.rebuildMacros.contains(depPkg)) {
            return false
        }
        let rootPkgName: String = getRootPkgName(depPkg)
        let cjoFile = Path(this.buildConfig.globalConfig.targetDir).join(rootPkgName).join("${depPkg}.cjo").toString()
        if (!fileExists(cjoFile) || !buildConfig.cjoTimeMap.contains(depPkg) || buildConfig.cjoTimeMap[depPkg] !=
            FileInfo(cjoFile).lastModificationTime.toUnixTimeStamp().toSeconds()) {
            return false
        }
        return true
    }

    func shouldNotify(downstreamTask: TaskIdentifier, task: TaskIdentifier, notification: Notification): Bool {
        let res = match ((notification, downstreamTask.compilePhase)) {
            case (CjoReady, LinkOnly) => false
            case (CjoReady, ObjOnly) => !taskMap[task].isMacro
            case (CjoReady, Default) =>
                !taskMap[task].isMacro &&
                !taskMap[downstreamTask].isLto &&
                !taskMap[downstreamTask].isMultiplatform &&
                taskMap[downstreamTask].outputType == OutputType.Static
            case (_, _) => true
        }
        if (res) {
            synchronized (notifyMutex) {
                if (!notified[task].add(downstreamTask)) {
                    return false
                }
            }
        }
        res
    }

    func printNotification(downstreamTask: TaskIdentifier, task: TaskIdentifier, notification: Notification): Unit {
        if (!buildConfig.isVerbose || !buildConfig.isCompilePipelineParallel) {
            return
        }
        println("Task `${taskId2String(task)}` notifies Task `${taskId2String(downstreamTask)}` with notification `${notification.toString()}`, waiting for ${scs[downstreamTask].count.toString()} tasks")
    }

    func notify(task: TaskIdentifier, notification: Notification): Unit {
        for (t in downstreamTasks[task] where shouldNotify(t, task, notification)) {
            if (let Err <- notification) {
                synchronized(buildMutex) {
                    shouldFail.add(t)
                }
            }
            scs[t].dec()
            printNotification(t, task, notification)
        }
    }

    func getCjo2TaskAndListenDirs(): (HashMap<String, TaskIdentifier>, HashSet<String>) {
        let cjo2Task = HashMap<String, TaskIdentifier>()
        let listenDirs = HashSet<String>()
        for (taskId in orderedTasks where taskId.compilePhase != LinkOnly) {
            let task = taskMap[taskId]
            listenDirs.add(task.targetPath)
            cjo2Task.add(Path(task.targetPath).join("${task.fullName}.cjo.flag").toString(), taskId)
        }
        (cjo2Task, listenDirs)
    }

    func action(task: CompileTask, taskId: TaskIdentifier, logger: BoundedFileLogger): Bool {
        synchronized(buildMutex) {
            if (shouldFail.contains(taskId)) {
                logger.release()
                return false
            }
        }

        if (task.checkDepCjo) {
            var nochange: Bool = true
            for (depPkg in task.requireTasks) {
                if (!checkNoChange(depPkg.fullName)) {
                    nochange = false
                    break
                }
            }

            for (subPkg in task.subPkgSet) {
                if (!checkNoChange(subPkg)) {
                    nochange = false
                    break
                }
            }

            if (nochange) {
                logger.release()
                return true
            }
        }

        if (!deleteLog(task)) {
            logger.release()
            return false
        }

        let cjcCall = constructCjcInvocation(task, this.buildConfig)
        let envBuilder = EnvironmentBuilder()
        envBuilder.prepend(LD_PATH, this.buildConfig.globalConfig.ldPath)
        specifyCjcJobs(cjcCall, this.buildConfig.cjcJobs)
        let res = runTask(task, cjcCall, envBuilder) 
        logger.log(task.outLogFile, task.errLogFile)
        res
    }

    func createWatchFSFuture(cjo2Task: HashMap<String, TaskIdentifier>, listenDirs: HashSet<String>): Future<Unit> {
        spawn { =>
            if (buildConfig.isCompilePipelineParallel) {
                watchFS(cjo2Task, listenDirs, notify, buildConfig.isVerbose)
            }
        }
    }

    func waitWatchFSFuture(watchFSFuture: Future<Unit>): Unit {
        if (buildConfig.isCompilePipelineParallel) {
            stopWatchFS(buildConfig.isVerbose)
        }
        watchFSFuture.get()
    }

    func build(): (Bool, HashSet<String>) {
        let logger = BoundedFileLogger(taskMap.size).start()

        let (cjo2Task, listenDirs) = getCjo2TaskAndListenDirs()
	    let watchFSFuture = createWatchFSFuture(cjo2Task, listenDirs)

        let sem = Semaphore(maxParallelSize)
        let futTasks = HashMap<TaskIdentifier, Future<Bool>>()
        // Guaranteed valid order follows 'buildConfig.packageList.all'
        for (taskId in orderedTasks) {
            sem.acquire()
            let task = taskMap[taskId]
            let future = spawn { =>
                scs[taskId].waitUntilZero()
                if (buildConfig.isVerbose) {
                    createAndWriteFile(task.outLogFile, "", mode: Append)
                }
                if (buildConfig.isVerbose && buildConfig.isCompilePipelineParallel) {
                    writeFile(task.outLogFile, "Task `${taskId2String(taskId)}` starts", Append)
                }
                let res = action(task, taskId, logger)
                sem.release()
                let notification = if (res) {
                    OUTPUTTYPE2NOTIFICATION.get(task.outputType.toString()).getOrThrow()
                } else { Err }
                notify(taskId, notification)
                if (buildConfig.isVerbose && buildConfig.isCompilePipelineParallel) {
                    let result = if (res) { "success" } else { "failure" }
                    writeFile(task.outLogFile, "Task `${taskId2String(taskId)}` ends in ${result}", Append)
                }
                res
            }

            futTasks.add(taskId, future)
        }

        var buildResult = true

        let successPkgs = HashSet<String>(futTasks.iterator().map { item => item[0].fullName } |> collectArray)
        for ((id, task) in futTasks) {
            if (!task.get()) {
                successPkgs.remove(id.fullName)
                buildResult = false
            }
        }

        if (!logger.unsafeWait()) {
            buildResult = false
        }

        waitWatchFSFuture(watchFSFuture)

        for ((cjoFlagFile, _) in cjo2Task) {
            if (!deleteFile(cjoFlagFile)) {
                buildResult = false
            }
        }

        (buildResult, successPkgs)
    }

    func deleteLog(task: CompileTask): Bool {
        var delLog: Bool = true
        if (!deleteFile(task.outLogFile)) {
            delLog = false
        }
        if (!deleteFile(task.errLogFile)) {
            delLog = false
        }
        return delLog
    }

    func runTask(compileTask: CompileTask, args: ArrayList<String>, envBuilder: EnvironmentBuilder): Bool {
        var execCmdFlag: Bool = true

        let env = getVariables()
        let commandStr = getCmdStr(envBuilder.asCliStrings(env), COMPILE_TOOL, args)
        if (this.buildConfig.isVerbose) {
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
}

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

private struct PrevStage {
    PrevStage(
        let chir!: String,
        let commonCjo!: String,
        let taskId!: TaskIdentifier
    ) { }
}

// Returns an array of compiler calls, which are required to compile a package
func createCompileTasks(r: ResolveItem, buildConfig: BuildConfig): ?ArrayList<CompileTask> {
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

    // Specific to the `--target` platform
    // `isNativeForCross == true` - package needs to be compiled with `host` target to be used by or is a `macro package` 
    func createForTarget(isNativeForCross!: Bool = false): ?ArrayList<CompileTask> {
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
        let srcDirToTaskId = HashMap<String, TaskIdentifier>()
        let srcDirToTargetPath = HashMap<String, String>()

        // Multiplatform packages need an arbitrary number of `cjc` calls
        // Non-multiplatform packages consist of single `sourceSetEntry`
        func createForSingleSourceSet(sourceSetEntry: CJMPPackageInfo): ?ArrayList<CompileTask> {
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

            let checkDepCjo = buildConfig.isIncremental && 
                r.outputType == Static &&
                buildConfig.indirectRebuilds.contains(r.fullName)

            let isLto = buildConfig.isLto && (outputType == Static || outputType == Exe)
            let ltoValue = if (isLto) { buildConfig.ltoValue } else { "" }

            let filename = if (outputType == Exe && buildConfig.packageList.exe.size <= 1 && !COMMON_INFO.inWorkspace) {
                buildConfig.exeName
            } else { fullName }

            let targetPath = if (outputType == Exe) {
                Path(targetDir).join(BIN).join(sourceSetDir).toString()
            } else {
                Path(targetDir).join(swapOrgName(rootPkgName)).join(sourceSetDir).toString()
            }
            let logPath = Path(targetDir).join(".build-logs").join(swapOrgName(rootPkgName)).join(sourceSetDir).toString()

            if (!createDirectory(targetPath) || !createDirectory(logPath)) {
                return None
            }

            let requireTasks = r.requires.iterator().map { name =>
                let isMacroName = buildConfig.packageList.isMacro(name)
                let waitTarget = if (isMacroName) { targetConfigName } else { target }
                let isSplittable = !isMultiplatform &&
                               !isLto &&
                               buildConfig.isCompilePipelineParallel &&
                               !isMacroName &&
                               (buildConfig.packageList.isDyLib(name) || buildConfig.packageList.isExe(name))
                let compilePhase = if (isSplittable) { ObjOnly } else { CompilePhase.Default }
                TaskIdentifier(name, waitTarget, "", compilePhase: compilePhase)
            } |> collectHashSet
            if (isMacro && isCrossCompile) {
                requireTasks.add(TaskIdentifier(fullName, targetConfigName, ""))
            }

            let isSplittable = !isMultiplatform &&
                           !isLto &&
                           buildConfig.isCompilePipelineParallel &&
                           !isMacro &&
                           (outputType == Dynamic || outputType == Exe)

            let compilePhase = if (isSplittable) { ObjOnly } else { CompilePhase.Default }

            let taskId = TaskIdentifier(
                fullName,
                target,
                if (!product) { sourceSetDir } else { "" },
                compilePhase: compilePhase
            )

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
                checkDepCjo: checkDepCjo,
                supressed: supressed,
                outLogFile: outLogFile,
                errLogFile: errLogFile,
                compileTargetOutput: if (isSplittable) { outputType } else { None }
            )

            let compileTasks = ArrayList<CompileTask>([task])
            if (!isSplittable) {
                return compileTasks
            }
            let linkTaskId = TaskIdentifier(
                fullName,
                target,
                if (!product) { sourceSetDir } else { "" },
                compilePhase: LinkOnly
            )
            let prevStageObj = Path(task.targetPath).join(getOutput(task).getOrThrow()).toString()
            let linkRequireTasks = getDepSet(fullName, buildConfig).iterator().filterMap { name =>
                    if(!buildConfig.packageList.isMacro(name) && !buildConfig.packageList.isTest(name)) {
                        let isSplittable = buildConfig.packageList.isDyLib(name) || buildConfig.packageList.isExe(name)
                        let compilePhase = if (isSplittable) { LinkOnly } else { CompilePhase.Default }
                        TaskIdentifier(name, target, "", compilePhase: compilePhase)
                    } else {
                        None
                    }
                } |> collectHashSet
            linkRequireTasks.add(task.id)
            let (linkOutLogFile, linkErrLogFile) = getLogFile(logPath, linkTaskId)
            let linkTask = CompileTask(
                id: linkTaskId,
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
                requireTasks: linkRequireTasks,
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
                checkDepCjo: checkDepCjo,
                supressed: supressed,
                outLogFile: linkOutLogFile,
                errLogFile: linkErrLogFile,
                prevStageObj: prevStageObj
            )
            compileTasks.add(linkTask)
            compileTasks
        }

        let sourceSetUnwrap = ArrayList<CompileTask>()
        for (sourceSetEntry in r.packagePath._sources) {
            let tasks = createForSingleSourceSet(sourceSetEntry)
            sourceSetUnwrap.add(all: tasks ?? return None)
        }
        sourceSetUnwrap
    }

    let result = ArrayList<CompileTask>()

    assertion { r.targetPlatform || r.nativePlatform }
    if (r.nativePlatform) {
        result.add(all: createForTarget(isNativeForCross: true) ?? return None)
    }
    if (r.targetPlatform) {
        result.add(all: createForTarget(isNativeForCross: false) ?? return None)
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

func printDependency(taskA: TaskIdentifier, taskB: TaskIdentifier, verbose: Bool): Unit {
    if (!verbose) {
        return
    }
    println("`${taskId2String(taskA)}` depends on `${taskId2String(taskB)}`")
}

func getDownstreamTasksAndSynCounters(buildConfig: BuildConfig, taskMap: TaskMap,
    orderedTasks: ArrayList<TaskIdentifier>): (DownStreamTasks, Task2SyncCounter) {
    let downstreamTasks = DownStreamTasks()
    let scs = Task2SyncCounter()
    for (taskId in orderedTasks where taskMap.contains(taskId)) {
        let depTasks = HashSet<TaskIdentifier>()
        downstreamTasks.addIfAbsent(taskId, HashSet<TaskIdentifier>())
        if (buildConfig.globalConfig.isProjectCombine && taskId.fullName == buildConfig.globalConfig.rootName) {
            for ((k, _) in taskMap where k != taskId && taskMap.contains(k)) {
                depTasks.add(k)
                downstreamTasks.addIfAbsent(k, HashSet<TaskIdentifier>())
                downstreamTasks[k].add(taskId)
                printDependency(taskId, k, buildConfig.isVerbose && buildConfig.isCompilePipelineParallel)
            }
        } else {
            let task = taskMap[taskId]
            for (dep in task.requireTasks where taskMap.contains(dep)) {
                depTasks.add(dep)
                downstreamTasks.addIfAbsent(dep, HashSet<TaskIdentifier>())
                downstreamTasks[dep].add(taskId)
                printDependency(taskId, dep, buildConfig.isVerbose && buildConfig.isCompilePipelineParallel)
            }

            for (depId in task.subPkgSet.iterator().map { dep => TaskIdentifier(dep, task.target, "") } where taskMap.contains(depId)) {
                depTasks.add(depId)
                downstreamTasks.addIfAbsent(depId, HashSet<TaskIdentifier>())
                downstreamTasks[depId].add(taskId)
                printDependency(taskId, depId, buildConfig.isVerbose && buildConfig.isCompilePipelineParallel)
            }
        }
        scs[taskId] = SyncCounter(depTasks.size)
    }
    (downstreamTasks, scs)
}

func parallelBuild(res: ModuleResolve, buildConfig: BuildConfig): Bool {
    let (taskMap, orderedTasks) = res.resolves.fold((TaskMap(), ArrayList<TaskIdentifier>())) {
        acc, r: ResolveItem =>
        if (buildConfig.isIncremental && !buildConfig.rebuildList.contains(r.fullName)) {
            return acc
        }
        if (!r.hasProdFiles) {
            buildConfig.rebuildList.remove(r.fullName)
            return acc
        }
        let compileTasks = createCompileTasks(r, buildConfig) ?? return acc
        for (task in compileTasks) {
            acc[0].add(task.id, task)
            acc[1].add(task.id)
        }
        acc
    }

    if (!taskMap.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let (downstreamTasks, scs) = getDownstreamTasksAndSynCounters(buildConfig, taskMap, orderedTasks)
 	 
 	let parallel = ParallelBuilder(buildConfig, taskMap, downstreamTasks, scs, orderedTasks.toArray())

    let (parallelResult, successPkgs) = parallel.build()

    for (r in res.resolves where successPkgs.contains(r.fullName)) {
        r.buildFlag = true
    }

    parallelResult
}

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
        case CjcParallelConfig.Spicific(v) =>
            args.add("-j${v}", at: 0)
    }
}

func calculateParallel(currentParalla: Int64): Int64 {
    var thisParallelSize = currentParalla
    let totalParallelSize = maxParallelSize

    if (currentParalla <= 2) {
        thisParallelSize = totalParallelSize
    } else {
        thisParallelSize = 2 * totalParallelSize / currentParalla
    }
    return thisParallelSize
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

func taskId2String(taskId: TaskIdentifier): String {
    "${taskId.fullName}|${taskId.target}|${taskId.sourceSetDir}|${taskId.compilePhase.toString()}"
}

func getLogFile(dir: String, taskId: TaskIdentifier): (String, String) {
    let logFileName = "${swapOrgName(taskId.fullName)}.${taskId.target}.${taskId.sourceSetDir}.${taskId.compilePhase.toString()}".replace("/", "#").replace("\\", "#")
    let outLogFile = Path(dir).join("${logFileName}.outlog").toString()
    let errLogFile = Path(dir).join("${logFileName}.errlog").toString()
    (outLogFile, errLogFile)
}

var watcherCb: (String) -> Unit = { _ => }

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
func watchFS(cjo2task: HashMap<String, TaskIdentifier>, listenDirs: HashSet<String>,
    notify: (TaskIdentifier, Notification) -> Unit, verbose: Bool): Unit {
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
func watchFS(_: HashMap<String, TaskIdentifier>, _: HashSet<String>,
    _: (TaskIdentifier, Notification) -> Unit, verbose: Bool): Unit {}

@When[(os != "iOS") && (env != "ohos") && (env != "android")]
func stopWatchFS(isVerbose: Bool): Unit {
    unsafe { stopFSWatcher(isVerbose) }
}

@When[(os == "iOS") || (env == "ohos") || (env == "android")]
func stopWatchFS(_: Bool): Unit {}