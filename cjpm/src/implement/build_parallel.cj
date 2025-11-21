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
import std.deriving.*

import cjpm.config.*
import cjpm.util.*

type TaskMap = HashMap<TaskIdentifier, ResolveConfig>

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

    public func start(): Unit {
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
            } catch (_: Exception){ 
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
    var buildConfig = BuildConfig()
    var taskMap = TaskMap()
    let buildMutex = Mutex()
    let successPkgs = HashMap<String, Bool>()

    init(buildConfig: BuildConfig) {
        this.buildConfig = buildConfig
    }

    func checkDepFuture(dep: TaskIdentifier, task: ResolveConfig, futTasks: HashMap<TaskIdentifier, Future<Bool>>,
        sem: Semaphore): Bool {
        var fut: Future<Bool>
        synchronized(this.buildMutex) {
            if (!futTasks.contains(dep)) {
                return true
            }
            fut = futTasks[dep]
        }
        return fut.get()
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

    func build(): Bool {
        let logger = BoundedFileLogger(this.taskMap.size)
        logger.start()

        let sem = Semaphore(maxParallelSize)
        let futTasks = HashMap<TaskIdentifier, Future<Bool>>()
        // Guaranteed valid order follows 'buildConfig.packageList.all'
        for (taskId in this.buildConfig.packageList.allIds) {
            if (!this.taskMap.contains(taskId)) {
                continue
            }

            sem.acquire()
            let task = this.taskMap[taskId]
            let future = spawn { =>
                if (this.buildConfig.globalConfig.isProjectCombine && taskId.fullName == buildConfig.globalConfig.rootName) {
                    for ((k,_) in this.taskMap where k != taskId) {
                        if (!this.checkDepFuture(k, task, futTasks, sem)) {
                            sem.release()
                            logger.release()
                            return false
                        }
                    }
                } else {
                    for (dep in task.requirePkgs) {
                        let depId = TaskIdentifier(dep, task.target)
                        if (!this.checkDepFuture(depId, task, futTasks, sem)) {
                            sem.release()
                            logger.release()
                            return false
                        }
                    }

                    for (dep in task.subPkgSet) {
                        let depId = TaskIdentifier(dep, task.target)
                        if (!this.checkDepFuture(depId, task, futTasks, sem)) {
                            sem.release()
                            logger.release()
                            return false
                        }
                    }
                }

                if (task.checkDepCjo) {
                    var nochange: Bool = true
                    for (depPkg in task.requirePkgs) {
                        if (!checkNoChange(depPkg)) {
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
                        sem.release()
                        logger.release()
                        return true
                    }
                }

                if (!deleteLog(task)) {
                    sem.release()
                    logger.release()
                    return false
                }



                let cjcCalls = constructCjcInvocations(task)
                let envBuilder = EnvironmentBuilder()
                envBuilder.prepend(LD_PATH, this.buildConfig.globalConfig.ldPath)

                var flag: Bool = true
                for (callArgs in cjcCalls where flag) {
                    flag = runTask(task, callArgs, envBuilder) 
                }
                sem.release()
                logger.log(task.outLogFile, task.errLogFile)
                return flag
            }

            synchronized(this.buildMutex) {
                futTasks.add(taskId, future)
            }
        }

        var isBuilt: Bool = true
        for ((id, task) in futTasks) {
            if (!task.get()) {
                isBuilt = false
                successPkgs.add(id.fullName, false)
            } else if (!successPkgs.contains(id.fullName)) {
                successPkgs.add(id.fullName, true)
            }
        }

        if (!logger.unsafeWait()) {
            isBuilt = false
        }

        return isBuilt
    }

    func deleteLog(task: ResolveConfig): Bool {
        var delLog: Bool = true
        if (!deleteFile(task.outLogFile)) {
            delLog = false
        }
        if (!deleteFile(task.errLogFile)) {
            delLog = false
        }
        return delLog
    }

    private func constructCjcInvocations(resolveConfig: ResolveConfig): ArrayList<ArrayList<String>> {
        let cjcCalls = ArrayList<ArrayList<String>>()
        if (resolveConfig.isMultiplatform) {
            cjcCalls.add(all: getMultiplatformPipeline(resolveConfig, this.buildConfig))
        } else {
            let args = getCommandArgs(resolveConfig, this.buildConfig, Default, resolveConfig.features)
            args.add(all: getTypeCommand(resolveConfig, this.buildConfig))
            cjcCalls.add(args)
        }
        return cjcCalls
    }

    func runTask(resolveConfig: ResolveConfig, args: ArrayList<String>, envBuilder: EnvironmentBuilder): Bool {
        var execCmdFlag: Bool = true
        CUR_PARALLEL_SIZE.fetchAdd(1)
        if (!futJudge(args)) {
            let jobs = calculateParallel(CUR_PARALLEL_SIZE.load())
            args.add("-j${jobs}", at: 0)
        }

        let env = getVariables()
        let commandStr = getCmdStr(envBuilder.asCliStrings(env), COMPILE_TOOL, args)
        if (this.buildConfig.isVerbose) {
            let verbose: String = "Compiling package `${resolveConfig.fullName}`: ${commandStr}\n"
            if (!createAndWriteFile(resolveConfig.outLogFile, verbose, mode: Append)) {
                return false
            }
        }

        if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
            addStartTime(resolveConfig.fullName, commandStr)
        }

        let outFilePath = resolveConfig.outLogFile
        let errFilePath = resolveConfig.errLogFile
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
                        moveCjcProfToCjpmDir(resolveConfig, buildConfig.globalConfig.compilePerformanceTargetDir)
                    }
                } else {
                    errFile.write("Error: failed to compile package `${resolveConfig.fullName}`, return code is ${returnCode}\n".toArray())
                    if (resolveConfig.isMultiplatform) {
                        errFile.write("${SourceLocationsPrinter(resolveConfig.packagePath, resolveConfig.features, project: resolveConfig.fullName)}".toArray())
                    }
                }
            } else {
                errFile.write("Error: failed to compile package `${resolveConfig.fullName}` with exception occurred\n".toArray())
            }
        } catch (e: Exception) {
            eprintln("Error: failed to write error log into ${errFilePath}: ${e.message}")
            execCmdFlag = false
        }
        CUR_PARALLEL_SIZE.fetchSub(1)
        outFile.close()
        errFile.close()

        if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
            addEndTime(resolveConfig.fullName)
        }
        return execCmdFlag
    }
}

func moveCjcProfToCjpmDir(resolveConfig: ResolveConfig, profcompilePerformancePath: String): Unit {
    let profSuffixArray: Array<String> = [".mem.prof", ".time.prof", ".info.prof"]
    for (profSuffix in profSuffixArray) {
        let pkgNameProfPath = Path(resolveConfig.targetPath).join(resolveConfig.fullName + profSuffix)
        if (exists(pkgNameProfPath)) {
            let profPath = Path(profcompilePerformancePath).join(resolveConfig.fullName + profSuffix)
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

func fromResolveItem(r: ResolveItem, buildConfig: BuildConfig): ?ArrayList<ResolveConfig> {
    func createResolveConfig(isNativeForCross!: Bool = false): ?ResolveConfig {
        let targetDir = if (isNativeForCross) { 
            buildConfig.globalConfig.nativeDir 
        } else { buildConfig.globalConfig.targetDir }
        let logPath = Path(targetDir).join(".build-logs").join(swapOrgName(r.rootPkgName)).toString()
        let outLogFile = Path(logPath).join("${swapOrgName(r.fullName)}.outlog").toString()
        let errLogFile = Path(logPath).join("${swapOrgName(r.fullName)}.errlog").toString()
        let targetPath = if (r.outputType == Exe && !buildConfig.requiredForTests) {
                Path(targetDir).join(BIN).toString()
            } else {
                Path(targetDir).join(swapOrgName(r.rootPkgName)).toString()
            }
        if (!createDirectory(targetPath) || !createDirectory(logPath)) {
            return None
        }

        if (r.isMultiplatform) {
            for (source in r.packagePath._sources) {
                let targetSpecificPath = Path(targetPath).join(source.outputSuffix).toString()
                if (!createDirectory(targetSpecificPath)) {
                    return None
                }
            }
        }

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

        let checkDepCjo = buildConfig.isIncremental && 
            r.outputType == Static &&
            buildConfig.indirectRebuilds.contains(r.fullName)

        let isCombined = r.superPkgCfg.isSome() && !isNativeForCross
        let isProjectCombined =
            buildConfig.globalConfig.isProjectCombine &&
            r.fullName == buildConfig.globalConfig.rootName &&
            !isNativeForCross

        var result = ResolveConfig()
        result.targetDir = targetDir
        result.targetPath = targetPath
        result.packagePath = r.packagePath
        result.rootPkgName = r.rootPkgName
        result.fullName = r.fullName
        result.outputType = r.outputType
        result.isCrossCompile = isCrossCompile
        result.target = target
        result.compileOption = compileOption
        result.overrideOption = overrideOption
        result.linkOption = linkOption
        result.customizedOption = r.customizedOption
        result.requirePkgs = r.requires
        result.outLogFile = outLogFile
        result.errLogFile = errLogFile
        result.checkDepCjo = checkDepCjo
        result.subPkgSet = r.superPkgCfg?.subPkgSet ?? HashSet<String>()
        result.isCombined = isCombined
        result.isProjectCombined = isProjectCombined
        result.hasTestFiles = r.hasTestFiles
        result.features = r.featureDeducer
            .addFeature(r.features)
            .collect()
        result.featureDeducer = r.featureDeducer
            .cleanFeatures() // leaving only mapping to apply them to source set features

        return result
    }

    let result = ArrayList<ResolveConfig>()

    assertion { r.targetPlatform || r.nativePlatform }
    if (r.targetPlatform) {
        result.add(createResolveConfig(isNativeForCross: false) ?? return None)
    }
    if (r.nativePlatform) {
        result.add(createResolveConfig(isNativeForCross: true) ?? return None)
    }

    return result
}

func parallelBuild(res: ModuleResolve, buildConfig: BuildConfig): Bool {
    let parallel = ParallelBuilder(buildConfig)

    res.forEachPkg { r: ResolveItem =>
        if (buildConfig.isIncremental && !buildConfig.rebuildList.contains(r.fullName)) {
            return true
        }
        if (!r.hasProdFiles) {
            buildConfig.rebuildList.remove(r.fullName)
            return true
        }
        let resolveConfigs = fromResolveItem(r, buildConfig) ?? return false
        for (config in resolveConfigs) {
            parallel.taskMap.add(TaskIdentifier.fromResolveConfig(config), config)
        }
        return true
    }

    if (!parallel.taskMap.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let parallelResult: Bool = parallel.build()

    for (r in res.resolves) {
        if (let Some(true) <- parallel.successPkgs.get(r.fullName)) {
            r.buildFlag = true
        }
    }

    return parallelResult
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
